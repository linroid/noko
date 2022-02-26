package com.linroid.noko

import com.linroid.noko.fs.FileSystem
import com.linroid.noko.types.*
import kotlinx.atomicfu.atomic
import kotlinx.atomicfu.locks.SynchronizedObject
import kotlinx.coroutines.Runnable
import kotlinx.coroutines.suspendCancellableCoroutine
import okio.Path
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
actual class Node actual constructor(
  private val cwd: String?,
  private val output: StdOutput,
  private val fs: FileSystem,
  keepAlive: Boolean,
  private val strictMode: Boolean,
){

  internal actual var pointer: Long = nativeNew(keepAlive, strictMode)
  private val listeners = HashSet<LifecycleListener>()

  private var running = atomic(false)
  private val lock = SynchronizedObject()
  private var global: JsObject? = null
  private var sequence = 0
  private val environmentVariables = HashMap(customEnvs)
  private val versions = HashMap(customVersions)

  internal actual val cleaner: (Long) -> Unit = { ref: Long ->
    check(ref != 0L) { "The reference has been already cleared" }
    // check(runtimePtr != 0L) { "The Node.js runtime is not active anymore" }
    nativeClearReference(ref)
  }

  internal lateinit var sharedNull: JsNull
  internal lateinit var sharedUndefined: JsUndefined
  internal lateinit var sharedTrue: JsBoolean
  internal lateinit var sharedFalse: JsBoolean

  /**
   * The thread that running node
   */
  private var thread: Thread? = null

  /**
   * Start node instance with arguments
   */
  actual fun start(vararg args: String) {
    val execArgs = ArrayList<String>()
    execArgs.add(EXEC_PATH)
    execArgs.addAll(args)
    sequence = counter.incrementAndGet()
    thread = startThread(isDaemon = true, name = "Node(${sequence})") {
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
  actual fun addListener(listener: LifecycleListener) = synchronized(lock) {
    listeners.add(listener)
  }

  /**
   * Removes a listener from this node instance
   */
  actual fun removeListener(listener: LifecycleListener) = synchronized(lock) {
    listeners.remove(listener)
  }

  /**
   * Determines if the process is currently active.  If it is inactive, either it hasn't
   * yet been started, or the process completed. Use aListener] to determine the state.
   *
   * @return true if active, false otherwise
   */
  private fun isRunning(): Boolean {
    return running.value && pointer != 0L
  }

  actual fun addEnv(key: String, value: String) {
    environmentVariables[key] = value
  }

  actual fun addVersion(key: String, value: String) {
    versions[key] = value
  }

  actual suspend fun <T> await(action: () -> T): T {
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
  actual fun exit(code: Int) {
    if (!isRunning()) {
      return
    }
    nativeExit(code)
    running.value = false
  }

  actual fun post(action: () -> Unit): Boolean {
    if (!isRunning()) {
      return false
    }
    if (isInNodeThread()) {
      action()
      return true
    }
    return nativePost(Runnable(action))
  }

  @Suppress("unused")
  private fun attach(global: JsObject) {
    this.global = global
    check(running.compareAndSet(false, update = true))
    check(isRunning()) { "isRunning() doesn't match the current state" }
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
      setEnv("PWD", cwd.toString())
      setupCode.append("process.chdir('${cwd}');\n")
    }
    if (output.supportsColor) {
      setEnv("COLORTERM", "truecolor")
    }
    environmentVariables.entries.forEach {
      setEnv(it.key, it.value)
    }
    setupCode.append("process.execPath = '$EXEC_PATH'\n")
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
  private fun detach(global: JsObject) {
    eventOnDetach(global)
  }

  internal actual fun checkThread() {
    if (strictMode) {
      check(isInNodeThread()) { "Operating js object is only allowed in origin thread: current=${currentThread()}" }
    }
  }

  internal actual fun isInNodeThread(): Boolean {
    return currentThread() === thread
  }

  private fun attachStdOutput(global: JsObject) {
    val process: JsObject = global.get("process")
    val stdout: JsObject = process.get("stdout")
    stdout.set("write", object : JsFunction(this@Node, "write") {
      override fun onCall(receiver: JsValue, parameters: Array<out JsValue>): JsValue? {
        output.stdout(parameters[0].toString())
        return null
      }
    })
    val stderr: JsObject = process.get("stderr")
    stderr.set("write", object : JsFunction(this@Node, "write") {
      override fun onCall(receiver: JsValue, parameters: Array<out JsValue>): JsValue? {
        output.stderr(parameters[0].toString())
        return null
      }
    })
  }

  private fun eventOnBeforeStart(global: JsObject) {
    fs.link(this)
    listeners.forEach {
      it.onNodeBeforeStart(this, global)
    }
  }

  private fun eventOnStart(global: JsObject) {
    listeners.forEach {
      it.onNodeStart(this, global)
    }
  }

  private fun eventOnDetach(global: JsObject) {
    listeners.forEach {
      it.onNodeBeforeExit(this, global)
    }
  }

  private fun eventOnExit(code: Int) {
    running.value = false
    listeners.forEach {
      it.onNodeExit(code)
    }
  }

  private fun eventOnError(error: JSException) {
    listeners.forEach {
      it.onNodeError(error)
    }
  }

  internal fun chroot(dir: Path) {
    nativeChroot(dir.toString())
  }

  internal fun mountFile(dst: Path, src: Path, mode: FileSystem.Mode) {
    nativeMountFile(src.toString(), dst.toString(), mode.flags)
  }

  @Throws(JSException::class)
  actual fun eval(code: String, source: String, line: Int): JsValue {
    return nativeEval(code, source, line)
  }

  @Throws(JSException::class)
  actual fun parseJson(json: String): JsValue {
    return nativeParseJson(json)
  }

  actual fun throwError(message: String): JsError {
    return nativeThrowError(message)
  }

  @Throws(JSException::class)
  @Deprecated("Not working")
  actual fun require(path: String): JsValue {
    return nativeRequire(path)
  }

  private external fun nativeNew(keepAlive: Boolean, strict: Boolean): Long
  private external fun nativeExit(exitCode: Int)
  private external fun nativeStart(args: Array<out String>): Int
  private external fun nativePost(action: Runnable): Boolean
  private external fun nativeMountFile(src: String, dst: String, mode: Int)
  private external fun nativeChroot(path: String)
  private external fun nativeEval(code: String, source: String, line: Int): JsValue
  private external fun nativeParseJson(json: String): JsValue
  private external fun nativeThrowError(message: String): JsError
  private external fun nativeRequire(path: String): JsObject
  private external fun nativeClearReference(ref: Long)

  companion object {
    private val counter = atomic(0)
    private val customVersions = HashMap<String, String>()
    private val customEnvs = HashMap<String, String>()
    private const val EXEC_PATH = "node"

    init {
      Platform.loadNativeLibraries()
    }

    fun addEnv(key: String, value: String) {
      customEnvs[key] = value
    }

    fun addVersion(name: String, version: String) {
      customVersions[name] = version
    }
  }
}
