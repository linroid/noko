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
import com.linroid.knode.path.FileSystem
import com.linroid.knode.path.RealFileSystem
import java.io.Closeable
import java.io.File
import java.lang.annotation.Native
import java.util.concurrent.atomic.AtomicInteger

/**
 * Create a new node instance
 *
 * @param cwd The current work directory for node
 * @param output The standard output interface
 * @param fs
 * @param keepAlive If all the js code is executed completely, should we keep the node running
 * @param strictMode If true to do thread checking when doing operation on js objects
 *
 * @author linroid
 * @since 2019-10-16
 */
@Keep
class KNode(
  private val cwd: File? = null,
  private val output: StdOutput,
  private val fs: FileSystem = RealFileSystem(),
  keepAlive: Boolean = false,
  private val strictMode: Boolean = true,
) : Closeable, Runnable {

  @Native
  private var ptr: Long = nativeNew(keepAlive, strictMode)
  private val listeners = HashSet<Listener>()

  @Volatile
  private var running = false
  private lateinit var context: JSContext
  private var sequence = 0
  private val environmentVariables = HashMap(customEnvs)
  private val versions = HashMap(customVersions)

  /**
   * The root directory for node.js, set by [chroot]
   */
  private var root: File? = null

  /**
   * The thread that running node
   */
  private var thread: Thread? = null

  /**
   * The execute arguments for node
   */
  private val execArgs = ArrayList<String>()

  /**
   * Start node instance with arguments
   */
  fun start(vararg args: String) {
    execArgs.clear()
    execArgs.add(exec.absolutePath)
    execArgs.addAll(args)
    sequence = counter.incrementAndGet()
    thread = Thread(this, "knode(${sequence})").also {
      it.isDaemon = true
      it.start()
    }
  }

  override fun run() {
    try {
      Log.d(TAG, "exec ${execArgs.joinToString(" ")}")
      val exitCode = nativeStart(execArgs.toTypedArray())
      Log.i(TAG, "node exited: $exitCode")
      eventOnStopped(exitCode)
    } catch (error: JSException) {
      eventOnError(error)
    } catch (error: Exception) {
      Log.w(TAG, "unexpected exception", error)
    }
  }

  /**
   * Add a listener to listen the state of node instance
   */
  fun addListener(listener: Listener) = synchronized(this) {
    listeners.add(listener)
  }

  /**
   * Removes a listener from this node instance
   */
  fun removeListener(listener: Listener) = synchronized(this) {
    listeners.remove(listener)
  }

  /**
   * Determines if the process is currently active.  If it is inactive, either it hasn't
   * yet been started, or the process completed. Use aListener] to determine the
   * state.
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

  /**
   * Instructs the VM to halt execution as quickly as possible
   * @param exitCode The exit code
   */
  fun exit(exitCode: Int) {
    Log.w(TAG, "exit($exitCode)", Exception())
    if (!isRunning()) {
      Log.w(TAG, "The node is ")
      Thread.dumpStack()
      return
    }
    nativeExit(exitCode)
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
    try {
      eventOnAttach(context)
    } catch (error: Throwable) {
      Log.w(TAG, "eventOnNodeBeforeStart", error)
    }

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


  /**
   * Change the root filesystem,
   * once you call this method, the filesystem for node will be changed to a virtual filesystem.
   * Call [mount] to mount addition files/directories and set custom permissions
   *
   * @param dir The directory path in the real filesystem
   */
  fun chroot(dir: File) {
    Log.d(TAG, "chroot $dir")
    this.root = dir
    nativeChroot(dir.absolutePath)
  }

  /**
   * Mount a [src] file/directory in the real filesystem as [dst] in the virtual file system
   *
   * @param src The directory path in the real filesystem
   * @param dst The destination path in the virtual filesystem
   * @param mode The file permission, see [FileAccessMode]
   * @param mapping If the [dst] path related to [root] is not exists in real filesystem,
   * whether to create one to make it visible when list files in it's parent directory
   */
  fun mount(src: File, dst: String, @FileAccessMode mode: Int, mapping: Boolean = true) {
    Log.d(TAG, "mount $src as $dst($mode), mapping=$mapping")
    check(src.exists()) { "File doesn't exists: $src, fs=$fs" }
    check(dst.startsWith("/")) { "The dst path must be a absolute path(starts with '/')" }

    val root = root ?: throw IllegalStateException("must call chroot first!")
    if (mapping) {
      if (src.isDirectory) {
        val dir = File(root, dst.substring(0))
        if (!dir.exists()) {
          dir.mkdirs()
        }
      } else {
        src.parentFile?.mkdirs()
        val file = File(root, dst.substring(0))
        if (!file.exists()) {
          file.createNewFile()
          file.setReadOnly()
        }
      }
    }

    nativeMount(src.absolutePath, dst, mode)
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

  private fun eventOnAttach(context: JSContext) {
    Log.i(TAG, "eventOnAttach()")
    fs.mount(context.node)
    listeners.forEach { it.onAttach(context) }
  }

  private fun eventOnPrepared(context: JSContext) {
    Log.i(TAG, "eventOnPrepared()")
    listeners.forEach {
      try {
        it.onPrepared(context)
      } catch (error: Throwable) {
        Log.w(TAG, "An error occurred in eventOnPrepared(), listener=$it", error)
        throw error
      }
    }
  }

  private fun eventOnDetach(context: JSContext) {
    Log.i(TAG, "onDetach()")
    listeners.forEach {
      try {
        it.onDetach(context)
      } catch (error: Throwable) {
        Log.w(TAG, "An error occurred in eventOnDetach(), listener=$it", error)
        throw error
      }
    }
  }

  private fun eventOnStopped(exitCode: Int) {
    Log.w(TAG, "eventOnStopped: exitCode=$exitCode")
    running = false
    listeners.forEach {
      try {
        it.onStopped(exitCode)
      } catch (error: Throwable) {
        Log.w(TAG, "An error occurred in onStopped(), listener=$it", error)
        throw error
      }
    }
  }

  private fun eventOnError(error: JSException) {
    Log.e(TAG, "eventOnError")
    listeners.forEach {
      try {
        it.onError(error)
      } catch (error: Throwable) {
        Log.w(TAG, "An error occurred in onError(), listener=$it", error)
        throw error
      }
    }
  }

  private external fun nativeNew(keepAlive: Boolean, strict: Boolean): Long

  private external fun nativeExit(exitCode: Int)

  private external fun nativeStart(args: Array<out String>): Int

  private external fun nativePost(action: Runnable): Boolean

  private external fun nativeMount(src: String, dst: String, mode: Int)

  private external fun nativeChroot(path: String)

  interface Listener {
    /**
     * The node is starting, do environment initialization work in this callback
     */
    fun onAttach(context: JSContext) {}

    /**
     * The node environment is ready, do anything you want
     */
    fun onPrepared(context: JSContext) {}

    /**
     * The node is going to stop
     */
    fun onDetach(context: JSContext) {}

    /**
     * The node has been stopped, it's time to cleanup resources
     */
    fun onStopped(exitCode: Int) {}

    /**
     * Whoops :( something went wrong
     */
    fun onError(error: JSException) {}
  }

  companion object {
    private const val TAG = "KNode"

    const val ACCESS_NONE = 0
    const val ACCESS_READ = 1
    const val ACCESS_WRITE = 2
    const val ACCESS_READ_WRITE = ACCESS_READ or ACCESS_WRITE

    const val ENV_HOME = "HOME"
    const val ENV_TMPDIR = "TMPDIR"

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
      Os.symlink(executableFile(context).absolutePath, exec.absolutePath)
    }

    fun executableFile(context: Context): File {
      return File(context.applicationInfo.nativeLibraryDir, "libnode_bin.so")
    }

    @JvmStatic
    private external fun nativeSetup(connectionManager: ConnectivityManager)
  }
}
