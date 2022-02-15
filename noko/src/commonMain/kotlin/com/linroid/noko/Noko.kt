package com.linroid.noko

import android.content.Context
import android.net.ConnectivityManager
import android.net.Network
import android.net.NetworkCapabilities
import android.net.NetworkRequest
import android.system.Os
import android.util.Log
import androidx.annotation.Keep
import com.google.gson.Gson
import com.linroid.noko.fs.FileMode
import com.linroid.noko.types.JSContext
import com.linroid.noko.types.JSFunction
import com.linroid.noko.types.JSObject
import com.linroid.noko.types.JSValue
import com.linroid.noko.fs.FileSystem
import com.linroid.noko.fs.RealFileSystem
import com.linroid.noko.ref.ReferenceWatcher
import kotlinx.coroutines.suspendCancellableCoroutine
import java.io.Closeable
import java.io.File
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
@Keep
class Noko(
  private val cwd: File? = null,
  private val output: StdOutput,
  private val fs: FileSystem = RealFileSystem(),
  keepAlive: Boolean = false,
  private val strictMode: Boolean = true,
) : Closeable {

  @Native
  private var ptr: Long = nativeNew(keepAlive, strictMode)
  private val listeners = HashSet<LifecycleListener>()

  @Volatile
  private var running = false
  private lateinit var context: JSContext
  private var sequence = 0
  private val environmentVariables = HashMap(customEnvs)
  private val versions = HashMap(customVersions)

  /**
   * The thread that running node
   */
  private var thread: Thread? = null

  /**
   * Start node instance with arguments
   */
  fun start(vararg args: String) {
    val execArgs = ArrayList<String>()
    execArgs.add(exec.absolutePath)
    execArgs.addAll(args)
    sequence = counter.incrementAndGet()
    thread = thread(isDaemon = true, name = "noko(${sequence})") {
      startInternal(execArgs.toTypedArray())
    }
  }

  private fun startInternal(execArgs: Array<String>) {
    try {
      Log.d(TAG, "exec ${execArgs.joinToString(" ")}")
      val exitCode = nativeStart(execArgs)
      Log.i(TAG, "node exited: $exitCode")
      eventOnExit(exitCode)
    } catch (error: JSException) {
      eventOnError(error)
    } catch (error: Exception) {
      Log.w(TAG, "unexpected exception", error)
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
    return running && ptr != 0L
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
    Log.w(TAG, "exit($code)", Exception())
    if (!isRunning()) {
      Log.w(TAG, "The node is ")
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
      Log.w(TAG, "Submit but not active: active=$running, ptr=$ptr, seq=$sequence", Exception())
      return false
    }
    if (isInNodeThread()) {
      action.run()
      return true
    }
    return nativePost(action)
  }

  @Suppress("unused")
  private fun attach(context: JSContext) {
    Log.i(TAG, "attach")
    this.context = context
    running = true
    context.node = this
    check(isRunning()) { "isActive() doesn't match the current state" }
    attachStdOutput(context)
    eventOnBeforeStart(context)
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
        context.eval(setupCode.toString())
      } catch (error: JSException) {
        Log.e(TAG, "Execute failed: stack=${error.stack()}", error)
        eventOnError(error)
      }
    }
    eventOnStart(context)
  }

  @Suppress("unused")
  private fun detach(context: JSContext) {
    Log.w(TAG, "detach()")
    eventOnDetach(context)
  }

  internal fun checkThread() {
    if (strictMode) {
      check(isInNodeThread()) { "Operating js object is only allowed in origin thread: current=${Thread.currentThread()}" }
    }
  }

  internal fun isInNodeThread(): Boolean {
    return Thread.currentThread() == thread
  }

  private fun attachStdOutput(context: JSContext) {
    Log.d(TAG, "attachStdOutput")
    val process: JSObject = context.get("process")
    val stdout: JSObject = process.get("stdout")
    stdout.set("write", object : JSFunction(context, "write") {
      override fun onCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue? {
        output.stdout(parameters[0].toString())
        return null
      }
    })
    val stderr: JSObject = process.get("stderr")
    stderr.set("write", object : JSFunction(context, "write") {
      override fun onCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue? {
        output.stderr(parameters[0].toString())
        return null
      }
    })
  }

  private fun eventOnBeforeStart(context: JSContext) {
    Log.i(TAG, "eventOnBeforeStart()")
    fs.link(this)
    listeners.forEach {
      it.onNodeBeforeStart(context)
    }
  }

  private fun eventOnStart(context: JSContext) {
    Log.i(TAG, "eventOnPrepared()")
    listeners.forEach {
      it.onNodeStart(context)
    }
  }

  private fun eventOnDetach(context: JSContext) {
    Log.i(TAG, "onDetach()")
    listeners.forEach {
      it.onNodeBeforeExit(context)
    }
  }

  private fun eventOnExit(code: Int) {
    Log.w(TAG, "eventOnExit: code=$code")
    running = false
    listeners.forEach {
      it.onNodeExit(code)
    }
  }

  private fun eventOnError(error: JSException) {
    Log.e(TAG, "eventOnError")
    listeners.forEach {
      it.onNodeError(error)
    }
  }

  internal fun chroot(dir: File) {
    nativeChroot(dir.absolutePath)
  }

  internal fun mountFile(dst: String, src: File, mode: FileMode) {
    nativeMountFile(src.absolutePath, dst, mode.flags)
  }

  private external fun nativeNew(keepAlive: Boolean, strict: Boolean): Long

  private external fun nativeExit(exitCode: Int)

  private external fun nativeStart(args: Array<out String>): Int

  private external fun nativePost(action: Runnable): Boolean

  private external fun nativeMountFile(src: String, dst: String, mode: Int)

  private external fun nativeChroot(path: String)


  companion object {
    private const val TAG = "Noko"

    private val counter = AtomicInteger(0)

    private val customVersions = HashMap<String, String>()
    private val customEnvs = HashMap<String, String>()
    private var exec = File("node")

    var gson: Gson = Gson()

    init {
      System.loadLibrary("noko")
    }

    private fun setDnsEnv(connectionManager: ConnectivityManager) {
      val network = connectionManager.activeNetwork

      val properties = connectionManager.getLinkProperties(network) ?: return
      if (properties.dnsServers.isNotEmpty()) {
        Os.setenv("DNS_SERVERS", properties.dnsServers.joinToString(",") {
          it.hostAddress ?: "8.8.8.8"
        }, true)
      } else {
        Os.unsetenv("DNS_SERVERS")
      }

      val domains = properties.domains
      Os.setenv("DNS_DOMAINS", domains ?: "", true)
    }

    fun setup(context: Context) {
      Log.i(TAG, "setup")
      ReferenceWatcher.start()

      val connectionManager =
        context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
      setDnsEnv(connectionManager)

      val request = NetworkRequest.Builder()
        .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
        .build()
      connectionManager.registerNetworkCallback(
        request,
        object : ConnectivityManager.NetworkCallback() {
          override fun onAvailable(network: Network) {
            Log.d(TAG, "network onAvailable: $network")
            setDnsEnv(connectionManager)
          }
        })
      nativeSetup(connectionManager)
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
          Log.e(TAG, str)
        }
      })
      node.start("-p", "process.versions")
    }

    @JvmStatic
    private external fun nativeSetup(connectionManager: ConnectivityManager)
  }
}
