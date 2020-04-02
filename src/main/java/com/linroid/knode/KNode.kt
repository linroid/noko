package com.linroid.knode

import android.util.Log
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
  keepAlive: Boolean = false
) : Closeable {

  @Native
  private var ptr: Long = nativeNew(keepAlive)
  private val listeners = HashSet<EventListener>()

  @Volatile
  private var active = false
  private lateinit var context: JSContext

  var thread: Thread? = null

  fun start(vararg args: String) {
    thread = thread(isDaemon = true, name = "knode-${seq.incrementAndGet()}") {
      try {
        val execArgs = ArrayList<String>()
        execArgs.add("node")
        execArgs.addAll(args)

        Log.d(TAG, execArgs.joinToString(" "))

        val exitCode = nativeStart(execArgs.toTypedArray())
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
    return active && ptr != 0L
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
      Log.w(TAG, "Submit but not active: active=$active, ptr=$ptr", Exception())
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
    // val process: JSObject = context.get("process")

    // process.set("argv0", "node")
    // process.set("argv", arrayOf("node", path, *argv))
    // process.set("execPath", execFile.absolutePath)
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
    customEnvs.entries.forEach {
      setEnv(it.key, it.value)
    }
    setupCode.append("process.execPath = '/node'\n")
    customVersions.forEach {
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
        if (BuildConfig.DEBUG) {
          Log.w(TAG, "stderr: " + parameters[0].toString())
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
    Log.e(TAG, "eventOnError")
    listeners.forEach { it.onNodeError(error) }
  }

  private external fun nativeNew(keepAlive: Boolean): Long

  private external fun nativeExit(exitCode: Int)

  private external fun nativeStart(args: Array<out String>): Int

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

    init {
      System.loadLibrary("knode")
      // if (BuildConfig.DEBUG) {
      //   Os.setenv("NODE_DEBUG", "*", true)
      // }
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
  }
}
