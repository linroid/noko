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
class KNode(private val pwd: File, private val output: StdOutput, private val supportColor: Boolean = false) : Closeable {

    @Native
    private var ptr: Long = nativeNew()
    private val listeners = HashSet<EventListener>()
    private var active = false
    private var done = false
    private lateinit var context: JSContext

    private lateinit var file: File
    private lateinit var argv: Array<out String>

    fun start(file: File, vararg argv: String) {
        this.file = file
        this.argv = argv
        thread(isDaemon = true, name = "knode-${seq.incrementAndGet()}") {
            val exitCode = nativeStart()
            eventOnExit(exitCode)
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
        attachStdOutput(context)
        val process: JSObject = context.get("process")
        val env: JSObject = process.get("env")
        val versions: JSObject = process.get("versions")
        active = true
        env.set("PWD", pwd.absolutePath)
        customEnvs.forEach { env.set(it.key, it.value) }
        if (supportColor) {
            env.set("COLORTERM", "truecolor")
        }
        process.set("argv0", "node")
        process.set("argv", arrayOf("node", file.absolutePath, *argv))
        customVersions.forEach {
            versions.set(it.key, it.value)
        }
        val chdir: JSFunction = process.get("chdir")
        chdir.call(process, JSString(context, pwd.absolutePath))
        eventOnPrepared(context)
        val cwdFunc: JSFunction = process.get("cwd")
        cwdFunc.call(process)
        val setupTTY = if (!supportColor) "" else """
            process.stderr.isTTY = true;
            process.stderr.isRaw = false;
            process.stdout.isTTY = true;
            process.stdout.isRaw = false;"""

        val script = """(() => {
            const fs = require('fs');
            const vm = require('vm');
            $setupTTY
            (new vm.Script(
            fs.readFileSync('${file.absolutePath}'),
            { filename: '${file.name}'} )).runInThisContext();
            })()
             """
        try {
            context.eval(script, file.absolutePath, 0)
        } catch (error: JSException) {
            Log.e(TAG, "Execute failed: file=${file.absolutePath}", error)
            close()
        }

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

    private external fun nativeNew(): Long

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
        private val seq = AtomicInteger(0)

        private val customVersions = HashMap<String, String>()
        private val customEnvs = HashMap<String, String>()
        var gson: Gson = Gson()

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
