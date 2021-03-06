package com.linroid.noko

import com.linroid.noko.fs.FileSystem
import com.linroid.noko.io.StandardIO
import com.linroid.noko.types.*
import kotlinx.atomicfu.atomic
import kotlinx.atomicfu.locks.SynchronizedObject
import kotlinx.coroutines.CoroutineDispatcher
import kotlinx.coroutines.InternalCoroutinesApi
import kotlinx.coroutines.Runnable
import kotlinx.coroutines.suspendCancellableCoroutine
import okio.Path
import kotlin.coroutines.CoroutineContext
import kotlin.coroutines.resume

actual class Node actual constructor(
  private val cwd: String?,
  private val fs: FileSystem,
  keepAlive: Boolean,
) {

  internal actual var pointer: Long = nativeNew(keepAlive)
  private val listeners = HashSet<LifecycleListener>()

  private var running = atomic(false)
  private val lock = SynchronizedObject()
  private var sequence = 0
  private val environmentVariables = HashMap(customEnvs)
  private val versions = HashMap(customVersions)

  actual var global: JsObject? = null
  actual var state: State = State.Initialized
  actual var stdio: StandardIO = StandardIO(this)
  actual val coroutineDispatcher: CoroutineDispatcher = object : CoroutineDispatcher() {
    override fun dispatch(context: CoroutineContext, block: Runnable) {
      check(post(block, true)) { "Unable to dispatch, Node's event loop is not available"}
    }

    override fun isDispatchNeeded(context: CoroutineContext): Boolean {
      return !isInEventLoop()
    }
  }

  internal actual val cleaner: (Long) -> Unit = { ref: Long ->
    check(ref != 0L) { "The reference has been already cleared" }
    // check(runtimePtr != 0L) { "The Node.js runtime is not active anymore" }
    nativeClearReference(ref)
  }

  internal lateinit var sharedUndefined: JsUndefined

  /**
   * The thread that running node
   */
  private var thread: Thread? = null

  init {
    require(cwd == null || cwd.isNotEmpty()) { "cwd must be null or non empty string" }
  }

  /**
   * Start node instance with arguments
   */
  actual fun start(vararg args: String) {
    val execArgs = ArrayList<String>()
    execArgs.add(EXEC_PATH)
    execArgs.addAll(args)
    sequence = counter.incrementAndGet()
    thread = startThread(isDaemon = true, name = "${sequence}.node") {
      startInternal(execArgs.toTypedArray())
    }
  }

  private fun startInternal(execArgs: Array<String>) {
    try {
      val exitCode = nativeStart(execArgs)
      onStop(exitCode)
    } catch (error: JsException) {
      onError(error)
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

  /**
   * Instructs the VM to halt execution as quickly as possible
   * @param code The exit code
   */
  actual fun exit(code: Int) {
    check(isRunning()) { "Node is not in running state" }
    nativeExit(code)
    running.value = false
  }

  actual fun post(action: Runnable, force: Boolean): Boolean {
    check(isRunning()) { "Node is not in running state" }
    if (!force && isInEventLoop()) {
      action.run()
      return true
    }
    return nativePost(action, force)
  }

  @Suppress("unused")
  private fun onAttach(global: JsObject) {
    this.global = global
    check(running.compareAndSet(false, update = true))
    check(isRunning()) { "Node is not in running state" }
    fs.link(this)
    // attachStdOutput(global)
    val setupCode = StringBuilder()
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
      } catch (error: JsException) {
        onError(error)
        return
      }
    }
    state = State.Attached
    listeners.forEach {
      it.onAttach(this, global)
    }
  }

  @Suppress("unused")
  private fun onStart(global: JsObject) {
    state = State.Started
    stdio.bind(global)
    listeners.forEach {
      it.onStart(this, global)
    }
  }

  @Suppress("unused")
  private fun onDetach(global: JsObject) {
    state = State.Detached
    listeners.forEach {
      it.onDetach(this, global)
    }
  }

  private fun onStop(code: Int) {
    running.value = false
    state = State.Stopped
    listeners.forEach {
      it.onStop(code)
    }
    dispose()
  }

  private fun onError(error: JsException) {
    state = State.Stopped
    listeners.forEach {
      it.onError(error)
    }
    dispose()
  }

  private fun dispose() {
    stdio.close()
  }

  actual fun isInEventLoop(): Boolean {
    return currentThread() === thread
  }

  internal actual fun chroot(dir: Path) {
    nativeChroot(dir.toString())
  }

  internal actual fun mountFile(dst: Path, src: Path, mode: FileSystem.Mode) {
    nativeMountFile(src.toString(), dst.toString(), mode.flags)
  }

  @Throws(JsException::class)
  actual fun eval(code: String, source: String, line: Int): Any? {
    return nativeEval(code, source, line)
  }

  @Throws(JsException::class)
  actual fun parseJson(json: String): JsValue {
    return nativeParseJson(json)
  }

  actual fun throwError(message: String): JsError {
    return nativeThrowError(message)
  }

  @Throws(JsException::class)
  actual fun require(path: String): JsValue {
    return nativeRequire(path)
  }

  private external fun nativeNew(keepAlive: Boolean): Long
  private external fun nativeExit(exitCode: Int)
  private external fun nativeStart(args: Array<out String>): Int
  private external fun nativePost(action: Runnable, force: Boolean): Boolean
  private external fun nativeMountFile(src: String, dst: String, mode: Int)
  private external fun nativeChroot(path: String)
  private external fun nativeEval(code: String, source: String, line: Int): Any?
  private external fun nativeParseJson(json: String): JsValue
  private external fun nativeThrowError(message: String): JsError
  private external fun nativeRequire(path: String): JsObject
  private external fun nativeClearReference(ref: Long)

  actual companion object {
    private val counter = atomic(0)
    private val customVersions = HashMap<String, String>()
    private val customEnvs = HashMap<String, String>()
    private const val EXEC_PATH = "node"
    private var inited = false

    init {
      Platform.loadNativeLibraries()
    }

    fun addEnv(key: String, value: String) {
      customEnvs[key] = value
    }

    fun addVersion(name: String, version: String) {
      customVersions[name] = version
    }

    actual fun setup(
      thread_pool_size: Int,
      connectivityManager: Any?,
    ) {
      check(!inited) { "Node.setup() has already been called" }
      require(thread_pool_size > 0) { "`thread_pool_size` must > 0" }
      inited = true
      nativeSetup(thread_pool_size, connectivityManager)
    }

    @JvmStatic
    private external fun nativeSetup(workerCount: Int, arg: Any?)
  }
}
