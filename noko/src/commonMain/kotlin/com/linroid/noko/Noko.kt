package com.linroid.noko

import com.linroid.noko.fs.FileMode
import com.linroid.noko.fs.FileSystem
import com.linroid.noko.fs.RealFileSystem
import com.linroid.noko.types.*
import kotlinx.coroutines.suspendCancellableCoroutine
import java.io.Closeable
import java.lang.annotation.Native
import java.util.concurrent.atomic.AtomicInteger
import kotlin.concurrent.thread
import kotlin.coroutines.resume

/**
 * Create a new Node.js instance
 *
 * @param cwd The current work directory for node
 * @param output The standard output interface
 * @param fs
 * @param keepAlive If all the js code is executed completely, should we keep the node running
 * @param strictMode If true to do thread checking when doing operation on js objects
 */
class Noko(
  private val cwd: String? = null,
  private val output: StdOutput,
  private val fs: FileSystem = RealFileSystem(),
  keepAlive: Boolean = false,
  private val strictMode: Boolean = true,
) : Closeable {

  @Native
  internal var nPtr: Long = nativeNew(keepAlive, strictMode)
  private val listeners = HashSet<LifecycleListener>()

  @Volatile
  private var running = false
  private var global: JSObject? = null
  private var sequence = 0
  private val environmentVariables = HashMap(customEnvs)
  private val versions = HashMap(customVersions)

  internal val cleaner: (Long) -> Unit = { ref: Long ->
    check(ref != 0L) { "The reference has been already cleared" }
    // check(runtimePtr != 0L) { "The Node.js runtime is not active anymore" }
    nativeClearReference(ref)
  }

  internal lateinit var sharedNull: JSNull
  internal lateinit var sharedUndefined: JSUndefined
  internal lateinit var sharedTrue: JSBoolean
  internal lateinit var sharedFalse: JSBoolean

  /**
   * The thread that running node
   */
  private var thread: Thread? = null

  /**
   * Start node instance with arguments
   */
  fun start(vararg args: String) {
    val execArgs = ArrayList<String>()
    execArgs.add(exec)
    execArgs.addAll(args)
    sequence = counter.incrementAndGet()
    thread = thread(isDaemon = true, name = "noko(${sequence})") {
      startInternal(execArgs.toTypedArray())
    }
  }

  private fun startInternal(execArgs: Array<String>) {
    try {
      val exitCode = nativeStart(execArgs)
      eventOnExit(exitCode)
    } catch (error: JSException) {
      eventOnError(error)
    } catch (error: Exception) {
      throw error
    }
  }

  /**
   * Add a listener to listen the state of node instance
   */
  fun addListener(lifecycleListener: LifecycleListener) = synchronized(this) {
    listeners.add(lifecycleListener)
  }

  /**
   * Removes a listener from this node instance
   */
  fun removeListener(lifecycleListener: LifecycleListener) = synchronized(this) {
    listeners.remove(lifecycleListener)
  }

  /**
   * Determines if the process is currently active.  If it is inactive, either it hasn't
   * yet been started, or the process completed. Use aListener] to determine the state.
   *
   * @return true if active, false otherwise
   */
  private fun isRunning(): Boolean {
    return running && nPtr != 0L
  }

  fun addEnv(key: String, value: String) {
    environmentVariables[key] = value
  }

  fun addVersion(key: String, value: String) {
    versions[key] = value
  }

  suspend fun <T> await(action: () -> T): T {
    return suspendCancellableCoroutine { continuation ->
      val success = post {
        try {
          continuation.resume(action())
        } catch (error: Exception) {
          continuation.cancel(error)
        }
      }
      if (!success) {
        continuation.cancel()
      }
    }
  }

  /**
   * Instructs the VM to halt execution as quickly as possible
   * @param code The exit code
   */
  fun exit(code: Int) {
    if (!isRunning()) {
      Thread.dumpStack()
      return
    }
    nativeExit(code)
    running = false
  }

  override fun close() {
    exit(0)
  }

  fun post(action: Runnable): Boolean {
    if (!isRunning()) {
      return false
    }
    if (isInNodeThread()) {
      action.run()
      return true
    }
    return nativePost(action)
  }

  @Suppress("unused")
  private fun attach(global: JSObject) {
    this.global = global
    running = true
    check(isRunning()) { "isActive() doesn't match the current state" }
    attachStdOutput(global)
    eventOnBeforeStart(global)
    val setupCode = StringBuilder()
    if (output.supportsColor) {
      setupCode.append(
        """
process.stderr.isTTY = true;
process.stderr.isRaw = true;
process.stdout.isTTY = true;
process.stdout.isRaw = true;
"""
      )
    }
    val setEnv: (key: String, value: String) -> Unit = { key, value ->
      setupCode.append("process.env['$key'] = '${value}';\n")
    }
    val setVersion: (key: String, value: String) -> Unit = { key, value ->
      setupCode.append("process.versions['$key'] = '${value}';\n")
    }

    cwd?.let {
      setEnv("PWD", fs.path(cwd))
      setupCode.append("process.chdir('${fs.path(cwd)}');\n")
    }
    if (output.supportsColor) {
      setEnv("COLORTERM", "truecolor")
    }
    environmentVariables.entries.forEach {
      setEnv(it.key, it.value)
    }
    setupCode.append("process.execPath = '${fs.path(exec)}'\n")
    versions.forEach {
      setVersion(it.key, it.value)
    }
    if (setupCode.isNotEmpty()) {
      try {
        eval(setupCode.toString())
      } catch (error: JSException) {
        eventOnError(error)
      }
    }
    eventOnStart(global)
  }

  @Suppress("unused")
  private fun detach(global: JSObject) {
    eventOnDetach(global)
  }

  internal fun checkThread() {
    if (strictMode) {
      check(isInNodeThread()) { "Operating js object is only allowed in origin thread: current=${Thread.currentThread()}" }
    }
  }

  internal fun isInNodeThread(): Boolean {
    return Thread.currentThread() === thread
  }

  private fun attachStdOutput(global: JSObject) {
    val process: JSObject = global.get("process")
    val stdout: JSObject = process.get("stdout")
    stdout.set("write", object : JSFunction(this@Noko, "write") {
      override fun onCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue? {
        output.stdout(parameters[0].toString())
        return null
      }
    })
    val stderr: JSObject = process.get("stderr")
    stderr.set("write", object : JSFunction(this@Noko, "write") {
      override fun onCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue? {
        output.stderr(parameters[0].toString())
        return null
      }
    })
  }

  private fun eventOnBeforeStart(global: JSObject) {
    fs.link(this)
    listeners.forEach {
      it.onNodeBeforeStart(this, global)
    }
  }

  private fun eventOnStart(global: JSObject) {
    listeners.forEach {
      it.onNodeStart(this, global)
    }
  }

  private fun eventOnDetach(global: JSObject) {
    listeners.forEach {
      it.onNodeBeforeExit(this, global)
    }
  }

  private fun eventOnExit(code: Int) {
    running = false
    listeners.forEach {
      it.onNodeExit(code)
    }
  }

  private fun eventOnError(error: JSException) {
    listeners.forEach {
      it.onNodeError(error)
    }
  }

  internal fun chroot(dir: String) {
    nativeChroot(dir)
  }

  internal fun mountFile(dst: String, src: String, mode: FileMode) {
    nativeMountFile(src, dst, mode.flags)
  }

  @Throws(JSException::class)
  fun eval(code: String, source: String = "", line: Int = 0): JSValue {
    return nativeEval(code, source, line)
  }

  @Throws(JSException::class)
  fun parseJson(json: String): JSValue {
    return nativeParseJson(json)
  }

  fun throwError(message: String): JSError {
    return nativeThrowError(message)
  }

  @Throws(JSException::class)
  @Deprecated("Not working")
  fun require(path: String): JSValue {
    return nativeRequire(path)
  }

  private external fun nativeNew(keepAlive: Boolean, strict: Boolean): Long
  private external fun nativeExit(exitCode: Int)
  private external fun nativeStart(args: Array<out String>): Int
  private external fun nativePost(action: Runnable): Boolean
  private external fun nativeMountFile(src: String, dst: String, mode: Int)
  private external fun nativeChroot(path: String)
  private external fun nativeEval(code: String, source: String, line: Int): JSValue
  private external fun nativeParseJson(json: String): JSValue
  private external fun nativeThrowError(message: String): JSError
  private external fun nativeRequire(path: String): JSObject
  private external fun nativeClearReference(ref: Long)

  companion object {
    private val counter = AtomicInteger(0)

    private val customVersions = HashMap<String, String>()
    private val customEnvs = HashMap<String, String>()
    private var exec = "node"

    init {
      System.loadLibrary("noko")
    }

    fun addEnv(key: String, value: String) {
      customEnvs[key] = value
    }

    fun addVersion(name: String, version: String) {
      customVersions[name] = version
    }

    fun versions(callback: (String) -> Unit) {
      val node = Noko(null, object : StdOutput {
        override fun stdout(str: String) {
          val trimmed = str.trim()
          if (trimmed.startsWith("{") && trimmed.endsWith("}")) {
            callback(str)
          }
        }

        override fun stderr(str: String) {
        }
      })
      node.start("-p", "process.versions")
    }
  }
}
