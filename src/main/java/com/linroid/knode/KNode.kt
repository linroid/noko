package com.linroid.knode

import android.os.Process
import android.os.Process.THREAD_PRIORITY_FOREGROUND
import android.util.Log
import androidx.annotation.Keep
import com.linroid.knode.js.*
import java.io.Closeable
import java.io.File

/**
 * @author linroid
 * @since 2019-10-16
 */
@Keep
class KNode(private val pwd: File, private val output: StdOutput) : Closeable {
    @Suppress
    private var ptr: Long = nativeInit()

    private val listeners = HashSet<EventListener>()
    private var active = false
    private var done = false
    private lateinit var context: JSContext

    private lateinit var file: File
    private lateinit var argv: Array<out String>

    fun start(file: File, vararg argv: String) {
        this.file = file
        this.argv = argv
        Thread({
            Process.setThreadPriority(THREAD_PRIORITY_FOREGROUND)
            val exitCode = nativeStart()
            eventOnExit(exitCode)
        }, "knode").start()
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
        return active && ::context.isInitialized
    }

    /**
     * Instructs the VM to halt execution as quickly as possible
     * @param exitCode The exit code
     */
    fun exit(exitCode: Int) {
        if (isActive() && ::context.isInitialized) {
            context.eval("process.exit($exitCode);")
        }
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

    fun mountFileSystem(obj: JSObject) {
        // setFs(obj.valueHash())
    }

    @Suppress("unused")
    private fun onBeforeStart(context: JSContext) {
        this.context = context
        Log.i(TAG, "onBeforeStart: context.toString()=$context")
        attachStdOutput(context)
        val process: JSObject = context.get("process")
        val env: JSObject = process.get("env")
        val versions: JSObject = process.get("versions")
        active = true
        env.set("PWD", pwd.absolutePath)
        envs.forEach { env.set(it.key, it.value) }
        process.set("argv0", "node")
        engineVersions.forEach {
            versions.set(it.key, it.value)
        }
        val chdir: JSFunction = process.get("chdir")
        chdir.call(process, JSString(context, pwd.absolutePath))

        val func = JSFunction(context, "test") { receiver, parameters ->
            Log.i(TAG, "call test($receiver, $parameters)")
            return@JSFunction JSString(context, "Hello World")
        }
        context.set("test", func)
        eventOnPrepared(context)
        val cwdFunc: JSFunction = process.get("cwd")
        val cwdRet = cwdFunc.call(process)
        Log.w(TAG, "cwdRet=${cwdRet}")
        val script = """(() => {
const fs = require('fs');
const vm = require('vm');  
(new vm.Script(
fs.readFileSync('${file.absolutePath}'),
{ filename: '${file.name}'} )).runInThisContext();
})()
 """
        context.eval(script, file.absolutePath, 0)
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

    @Suppress("unused")
    private fun onBeforeExit(exitCode: Int) {
        active = false
        done = true
        nativeDispose()
        eventOnExit(exitCode)
    }

    private fun eventOnPrepared(context: JSContext) {
        Log.i(TAG, "eventOnPrepared")
        listeners.forEach { it.onNodePrepared(context) }
    }

    private fun eventOnFinished(exitCode: Int) {
        Log.i(TAG, "eventOnFinished: exitCode=$exitCode")
        listeners.forEach { it.onNodeFinished(exitCode) }
    }

    private fun eventOnExit(exitCode: Int) {
        Log.w(TAG, "eventOnExit: exitCode=$exitCode")
        listeners.forEach { it.onNodeExited(exitCode) }
    }

    private fun eventOnError(error: JSException) {
        Log.e(TAG, "eventOnError}")
        listeners.forEach { it.onNodeError(error) }
    }

    private external fun nativeInit(): Long

    private external fun nativeStart(): Int

    private external fun nativeDispose()

    private external fun nativeSetFs(fsPtr: Long)

    interface EventListener {
        fun onNodePrepared(context: JSContext)

        fun onNodeFinished(exitCode: Int)

        fun onNodeExited(exitCode: Int)

        fun onNodeError(error: JSException)
    }

    companion object {
        private const val TAG = "KNode"

        private val engineVersions = HashMap<String, String>()
        private val envs = HashMap<String, String>()

        init {
            System.loadLibrary("knode")
        }

        fun setEnv(key: String, value: String) {
            envs[key] = value
        }

        fun setEngine(name: String, version: String) {
            engineVersions[name] = version
        }
    }
}
