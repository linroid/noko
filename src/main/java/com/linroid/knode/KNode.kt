package com.linroid.knode

import android.content.Context
import android.net.ConnectivityManager
import android.net.Network
import android.net.NetworkCapabilities
import android.net.NetworkRequest
import android.system.Os
import android.util.Log
import androidx.annotation.IntDef
import androidx.annotation.Keep
import com.google.gson.Gson
import com.linroid.knode.js.JSContext
import com.linroid.knode.js.JSFunction
import com.linroid.knode.js.JSObject
import com.linroid.knode.js.JSValue
import java.io.Closeable
import java.io.File
import java.lang.annotation.Native
import java.util.concurrent.atomic.AtomicInteger
import kotlin.concurrent.thread

/**
 * @author linroid
 * @since 2019-10-16
 */
@Keep
class KNode(
  private val cwd: File? = null,
  private val output: StdOutput,
  keepAlive: Boolean = false,
  val strict: Boolean = false,
) : Closeable {

  @Native
  private var ptr: Long = nativeNew(keepAlive, strict)
  private val listeners = HashSet<EventListener>()

  @Volatile
  private var active = false
  private lateinit var context: JSContext
  private var seq = 0
  private val envs = HashMap(customEnvs)
  private val versions = HashMap(customVersions)

  private var thread: Thread? = null

  fun start(vararg args: String) {
    seq = counter.incrementAndGet()
    thread = thread(isDaemon = true, name = "knode(${seq})") {
      try {
        val execArgs = ArrayList<String>()
        execArgs.add(exec.absolutePath)
        execArgs.addAll(args)

        Log.d(TAG, execArgs.joinToString(" "))

        val exitCode = nativeStart(execArgs.toTypedArray())
        Log.i(TAG, "node exited: $exitCode")
        eventOnExited(exitCode)
      } catch (error: JSException) {
        eventOnError(error)
      }
    }
  }

  fun addEventListener(listener: EventListener) = synchronized(this) {
    listeners.add(listener)
  }

  /**
   * Removes a listener from this Process
   * @param listener the listener interface object to remove
   */
  fun removeEventListener(listener: EventListener) = synchronized(this) {
    listeners.remove(listener)
  }

  /**
   * Determines if the process is currently active.  If it is inactive, either it hasn't
   * yet been started, or the process completed. Use an @EventListener to determine the
   * state.
   * @return true if active, false otherwise
   */
  private fun isActive(): Boolean {
    return active && ptr != 0L
  }

  fun addEnv(key: String, value: String) {
    envs[key] = value
  }

  fun addVersion(key: String, value: String) {
    versions[key] = value
  }

  /**
   * Instructs the VM to halt execution as quickly as possible
   * @param exitCode The exit code
   */
  fun exit(exitCode: Int) {
    Log.w(TAG, "exit($exitCode)")
    if (!isActive()) {
      Log.w(TAG, "dispose but not active")
      Thread.dumpStack()
      return
    }
    nativeExit(exitCode)
    context.runtimePtr = 0L
    active = false
  }

  override fun close() {
    exit(0)
  }

  fun post(action: Runnable): Boolean {
    if (!isActive()) {
      Log.w(TAG, "Submit but not active: active=$active, ptr=$ptr, seq=$seq", Exception())
      return false
    }
    if (isInThread()) {
      action.run()
      return true
    }
    return nativePost(action)
  }

  @Suppress("unused")
  private fun onBeforeStart(context: JSContext) {
    Log.i(TAG, "onBeforeStart")
    this.context = context
    active = true
    context.node = this
    check(isActive()) { "isActive is not match the current state" }
    attachStdOutput(context)
    // val process: JSObject = context.get("process")

    // process.set("argv0", "node")
    // process.set("argv", arrayOf("node", path, *argv))
    // process.set("execPath", execFile.absolutePath)
    val setupCode = StringBuilder()
    if (output.supportsColor) {
      setupCode.append("""
process.stderr.isTTY = true;
process.stderr.isRaw = true;
process.stdout.isTTY = true;
process.stdout.isRaw = true;
"""
      )
    }
    // setupCode.append("process.argv0='node';\n")
    // setupCode.append("process.argv[0]='node';\n")
    val setEnv: (key: String, value: String) -> Unit = { key, value ->
      setupCode.append("process.env['$key'] = '${value}';\n")
    }
    val setVersion: (key: String, value: String) -> Unit = { key, value ->
      setupCode.append("process.versions['$key'] = '${value}';\n")
    }

    cwd?.let {
      setEnv("PWD", cwd.absolutePath)
      setupCode.append("process.chdir('${cwd.absolutePath}');\n")
    }
    if (output.supportsColor) {
      setEnv("COLORTERM", "truecolor")
    }
    envs.entries.forEach {
      setEnv(it.key, it.value)
    }
    setupCode.append("process.execPath = '/node'\n")
    versions.forEach {
      setVersion(it.key, it.value)
    }
    if (setupCode.isNotEmpty()) {
      try {
        // setupCode.append("const vm = require('vm');\n")
        // setupCode.append("(new vm.Script(`${setupCode}`)).runInThisContext();\n")
        if (BuildConfig.DEBUG) {
          Log.d(TAG, "setupCode: \n$setupCode")
        }
        context.eval(setupCode.toString())
      } catch (error: JSException) {
        Log.e(TAG, "Execute failed: stack=${error.stack()}", error)
        eventOnError(error)
      }
    }
    eventOnPrepared(context)
  }


  fun chroot(path: String) {
    Log.d(TAG, "chroot $path")
    nativeChroot(path)
  }

  fun mount(source: String, target: String, mode: Int) {
    Log.d(TAG, "mount $source as $target($mode)")
    nativeMount(source, target, mode)
  }

  @Suppress("unused")
  private fun onBeforeExit(context: JSContext) {
    Log.w(TAG, "onBeforeExit")
    eventOnBeforeExit(context)
  }

  internal fun checkThread() {
    if (strict) {
      check(!isInThread()) { "Operating js object is only allowed in origin thread: current=${Thread.currentThread()}" }
    }
  }

  fun isInThread(): Boolean {
    return Thread.currentThread() != thread
  }

  private fun attachStdOutput(context: JSContext) {
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

  private fun eventOnPrepared(context: JSContext) {
    Log.i(TAG, "eventOnPrepared")
    listeners.forEach { it.onNodePrepared(context) }
  }

  private fun eventOnBeforeExit(context: JSContext) {
    Log.i(TAG, "eventOnBeforeExit")
    listeners.forEach { it.onNodeBeforeExit(context) }
  }

  private fun eventOnExited(exitCode: Int) {
    Log.w(TAG, "eventOnExited: exitCode=$exitCode")
    active = false
    listeners.forEach { it.onNodeExited(exitCode) }
  }

  private fun eventOnError(error: JSException) {
    Log.e(TAG, "eventOnError")
    listeners.forEach { it.onNodeError(error) }
  }

  private external fun nativeNew(keepAlive: Boolean, strict: Boolean): Long

  private external fun nativeExit(exitCode: Int)

  private external fun nativeStart(args: Array<out String>): Int

  private external fun nativePost(action: Runnable): Boolean

  private external fun nativeMount(source: String, target: String, mode: Int)

  private external fun nativeChroot(path: String)

  interface EventListener {

    fun onNodePrepared(context: JSContext) {}

    fun onNodeBeforeExit(context: JSContext) {}

    fun onNodeExited(exitCode: Int) {}

    fun onNodeError(error: JSException) {}
  }

  companion object {
    private const val TAG = "KNode"

    const val ACCESS_NONE = 0
    const val ACCESS_READ = 1
    const val ACCESS_WRITE = 2
    const val ACCESS_READ_WRITE = ACCESS_READ or ACCESS_WRITE

    @IntDef(ACCESS_NONE, ACCESS_READ, ACCESS_WRITE, ACCESS_READ_WRITE)
    annotation class FileAccessMode

    private val counter = AtomicInteger(0)

    private val customVersions = HashMap<String, String>()
    private val customEnvs = HashMap<String, String>()
    private var exec = File("node")

    var gson: Gson = Gson()

    init {
      System.loadLibrary("knode")
      // if (BuildConfig.DEBUG) {
      //   Os.setenv("NODE_DEBUG", "*", true)
      // }
    }

    private fun setDnsEnv(connectionManager: ConnectivityManager) {
      val network = connectionManager.activeNetwork

      val properties = connectionManager.getLinkProperties(network) ?: return
      if (properties.dnsServers.isNotEmpty()) {
        Os.setenv("DNS_SERVERS", properties.dnsServers.joinToString(",") { it.hostAddress }, true)
      } else {
        Os.unsetenv("DNS_SERVERS")
      }

      val domains = properties.domains
      Os.setenv("DNS_DOMAINS", domains ?: "", true)
    }

    fun setup(context: Context) {
      Log.i(TAG, "setup")
      ReferenceWatcher.start()

      val connectionManager = context.getSystemService(Context.CONNECTIVITY_SERVICE) as ConnectivityManager
      setDnsEnv(connectionManager)

      val request = NetworkRequest.Builder()
        .addTransportType(NetworkCapabilities.TRANSPORT_CELLULAR)
        .build()
      connectionManager.registerNetworkCallback(request, object : ConnectivityManager.NetworkCallback() {
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
      val node = KNode(null, object : StdOutput {
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

    fun mountExecutable(context: Context, exec: File) {
      this.exec = exec

      val libraryDir = context.applicationInfo.nativeLibraryDir

      val path = Os.getenv("PATH").orEmpty()
      Os.setenv("PATH", "$path:${libraryDir}", true)

      val libraryPath = Os.getenv("LD_LIBRARY_PATH").orEmpty()
      Os.setenv("LD_LIBRARY_PATH", "$libraryPath:${libraryDir}", true)

      Os.setenv("LD_PRELOAD", "${libraryDir}/libexec.so", true)

      // Create symlink to execute node cmd
      exec.delete()
      Os.symlink("$libraryDir/node.so", exec.absolutePath)
    }

    @JvmStatic
    private external fun nativeSetup(connectionManager: ConnectivityManager)
  }
}
