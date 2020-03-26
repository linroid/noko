package com.linroid.knode

import android.util.Log
import androidx.annotation.Keep
import com.google.gson.Gson
import com.linroid.knode.js.*
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
class KNode(private val pwd: File, private val output: StdOutput) : Closeable {

  @Native
  private var ptr: Long = nativeNew()
  private val listeners = HashSet<EventListener>()
  @Volatile
  private var active = false
  private lateinit var context: JSContext

  private lateinit var path: String
  private lateinit var argv: Array<out String>
  var thread: Thread? = null

  fun start(path: String, vararg argv: String) {
    this.path = path
    this.argv = argv
    thread = thread(isDaemon = true, name = "knode-${seq.incrementAndGet()}") {
      try {
        val exitCode = nativeStart()
        Log.i(TAG, "node exited: $exitCode")
        eventOnExit(exitCode)
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
    return active && ::context.isInitialized && ptr != 0L
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
    submit(Runnable {
      Log.d(TAG, "eval process.exit($exitCode)")
      context.eval("process.exit($exitCode);")
    })
    active = false
  }

  // /**
  //  * Instructs the VM not to shutdown the process when no more callbacks are pending.  In effect,
  //  * this method indefinitely leaves a callback pending until the resulting
  //  * #org.liquidplayer.javascript.JSContextGroup.LoopPreserver is released.  The loop preserver
  //  * must eventually be released or the process will remain active indefinitely.
  //  * @return A preserver object
  //  */
  // fun keepAlive(): JSContextGroup.LoopPreserver? {
  //     val ctx = context.get()
  //     return if (isActive() && ctx != null) {
  //         ctx.group!!.keepAlive()
  //     } else null
  // }

  override fun close() {
    exit(0)
  }

  fun mountFileSystem(fs: VirtualFileSystem) {
    nativeMountFileSystem(fs.thiz)
  }

  fun newFileSystem(): VirtualFileSystem {
    val thiz = nativeNewFileSystem()
    return VirtualFileSystem(thiz)
  }

  fun submit(action: Runnable): Boolean {
    if (!isActive()) {
      val isInitialized = this::context::isInitialized
      Log.w(TAG, "Submit but not active: active=$active, isInitialized=${isInitialized}, ptr=$ptr", Exception())
      return false
    }
    if (Thread.currentThread() == thread) {
      action.run()
      return true
    }
    return nativeSubmit(action)
  }

  @Suppress("unused")
  private fun onBeforeStart(context: JSContext) {
    Log.i(TAG, "onBeforeStart")
    this.context = context
    active = true
    context.node = this
    check(isActive()) { "isActive is not match the current state" }
    attachStdOutput(context)
    val process: JSObject = context.get("process")
    val env: JSObject = process.get("env")
    val versions: JSObject = process.get("versions")
    env.set("PWD", pwd.absolutePath)
    customEnvs.forEach { env.set(it.key, it.value) }
    if (output.supportsColor) {
      env.set("COLORTERM", "truecolor")
    }
    process.set("argv0", "node")
    process.set("argv", arrayOf("node", path, *argv))
    customVersions.forEach {
      versions.set(it.key, it.value)
    }
    // process.set("execPath", execFile.absolutePath)
    val chdir: JSFunction = process.get("chdir")
    chdir.call(process, JSString(context, pwd.absolutePath))
    eventOnPrepared(context)
    val setupCode = StringBuilder()
    if (output.supportsColor) {
      setupCode.append(
        """process.stderr.isTTY = true;
            process.stderr.isRaw = true;
            process.stdout.isTTY = true;
            process.stdout.isRaw = true;"""
      )
    }
    // val script = """(() => {
    //     const fs = require('fs');
    //     const vm = require('vm');
    //     $setupTTY
    //     (new vm.Script(
    //     fs.readFileSync('${file.absolutePath}'),
    //     { filename: '${file.name}'} )).runInThisContext();
    //     })()
    //      """
    setupCode.append("require('${path}');")
    try {
      context.eval(setupCode.toString(), path, 0)
    } catch (error: JSException) {
      Log.e(TAG, "Execute failed: file=${path}, stack=${error.stack()}", error)
      eventOnError(error)
    }
  }

  private fun attachStdOutput(context: JSContext) {
    val process: JSObject = context.get("process")
    val stdout: JSObject = process.get("stdout")
    stdout.set("write", object : JSFunction(context, "write") {
      override fun onCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue? {
        if (BuildConfig.DEBUG) {
          Log.d(TAG, parameters[0].toString())
        }
        output.stdout(parameters[0].toString())
        return null
      }
    })
    val stderr: JSObject = process.get("stderr")
    stderr.set("write", object : JSFunction(context, "write") {
      override fun onCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue? {
        if (BuildConfig.DEBUG) {
          Log.w(TAG, parameters[0].toString())
        }
        output.stderr(parameters[0].toString())
        return null
      }
    })
  }

  private fun eventOnPrepared(context: JSContext) {
    Log.i(TAG, "eventOnPrepared")
    listeners.forEach { it.onNodePrepared(context) }
  }

  private fun eventOnExit(exitCode: Int) {
    Log.w(TAG, "eventOnExit: exitCode=$exitCode")
    active = false
    listeners.forEach { it.onNodeExited(exitCode) }
  }

  private fun eventOnError(error: JSException) {
    Log.e(TAG, "eventOnError}")
    listeners.forEach { it.onNodeError(error) }
  }

  private external fun nativeNew(): Long

  private external fun nativeStart(): Int

  private external fun nativeNewFileSystem(): JSObject

  private external fun nativeMountFileSystem(obj: JSObject)

  private external fun nativeSubmit(action: Runnable): Boolean

  interface EventListener {

    fun onNodePrepared(context: JSContext) {}

    fun onNodeExited(exitCode: Int) {}

    fun onNodeError(error: JSException) {}
  }

  companion object {
    private const val TAG = "KNode"
    private val seq = AtomicInteger(0)

    private val customVersions = HashMap<String, String>()
    private val customEnvs = HashMap<String, String>()
    var gson: Gson = Gson()
    var execFile: File = File("/bin/node")

    init {
      System.loadLibrary("knode")
    }

    fun addEnv(key: String, value: String) {
      customEnvs[key] = value
    }

    fun addVersion(name: String, version: String) {
      customVersions[name] = version
    }
  }
}
