package com.linroid.knode

import android.util.Log
import androidx.annotation.Keep
import com.linroid.knode.js.JSException
import com.linroid.knode.js.JSObject
import com.linroid.knode.js.JSContext
import java.io.Closeable
import java.io.File
import java.lang.ref.WeakReference
import android.os.Process
import android.os.Process.THREAD_PRIORITY_FOREGROUND

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
    private var context = WeakReference<JSContext>(null)

    private lateinit var file: File
    private lateinit var argv: Array<out String>

    fun start(file: File, vararg argv: String) {
        this.file = file
        this.argv = argv
        Thread({
            Process.setThreadPriority(THREAD_PRIORITY_FOREGROUND)
            // val exitCode = start(arrayOf(file.absolutePath, *argv))
            val exitCode = start(arrayOf("-e", "global.__beforeStart();"))
            eventOnExit(exitCode)
        }, "node").start()
    }

    fun addEventListener(listener: EventListener) = synchronized(this) {
        listeners.add(listener)
        // val ctx = context.get()
        // if (isActive() && ctx != null) {
        //     ctx.sync(Runnable {
        //         if (isActive()) {
        //             listener.onNodeBooted(ctx)
        //         }
        //     })
        // }
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
    fun isActive(): Boolean {
        return active && context.get() != null
    }

    /**
     * Instructs the VM to halt execution as quickly as possible
     * @param exitCode The exit code
     */
    fun exit(exitCode: Int) {
        val ctx = context.get()
        if (isActive() && ctx != null) {
            ctx.eval("process.exit($exitCode);")
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

    private var holdContext: JSObject? = null

    @Suppress("unused")
    private fun onBeforeStart(context: JSContext) {
        val process = context.get("process")
        Log.i(TAG, "onBeforeStart: context.toString()=$context, process.toJson()=${process.toJson()}")
        active = true
        // val ctx = holdContext ?: return
        // context = WeakReference(ctx)
        // ctx.property(READY_METHOD_NAME, object : JSFunction(ctx, READY_METHOD_NAME) {
        //     @Suppress("unused")
        //     fun __onNodeReady() {
        //         if (!isActive()) {
        //             return
        //         }
        //         ctx.deleteProperty(READY_METHOD_NAME)
        //         attachOutput(ctx)
        //
        //         eventOnBooted(ctx)
        //
        //         val onExitFunc = object : JSFunction(ctx, "onExit") {
        //             @SuppressWarnings("unused")
        //             fun onExit(code: Long) {
        //                 eventOnFinished(code)
        //             }
        //         }
        //         JSFunction(ctx, "__onExit", arrayOf("exitFunc"), "process.on('exit', exitFunc);", null, 0)
        //             .call(null, onExitFunc)
        //
        //         val onUncaughtException = object : JSFunction(ctx, "onUncaughtException") {
        //             @SuppressWarnings("unused")
        //             fun onUncaughtException(error: JSObject) {
        //                 eventOnError(JSException(error))
        //                 ctx.evaluateScript("process.exit(process.exitCode === undefined ? -1 : process.exitCode)")
        //             }
        //         }
        //         JSFunction(ctx, "__onUncaughtException", arrayOf("handleFunc"), "process.on('uncaughtException',handleFunc);", null, 0)
        //             .call(null, onUncaughtException)
        //
        //
        //         val process = ctx.property("process").toObject()
        //         if (engineVersions.size > 0) {
        //             val versions = process.property("versions").toObject()
        //             engineVersions.forEach { versions.property(it.key, it.value) }
        //         }
        //
        //         val env = process.property("env").toObject()
        //         envs.forEach { env.property(it.key, it.value) }
        //         env.property("PWD", pwd.absolutePath)
        //         // env.property("_", "/bin/node")
        //
        //
        //         process.property("argv", arrayOf("node", file.absolutePath, *argv))
        //
        //
        //         val script = "(() => {" +
        //                 "  process.chdir(\"${pwd.absolutePath}\");" +
        //                 "  const fs = require('fs'), vm = require('vm'); " +
        //                 "  (new vm.Script(fs.readFileSync('${file.absolutePath}'), " +
        //                 "     {filename: '${file.name}'} )).runInThisContext();" +
        //                 "})()"
        //         ctx.evaluateScript(script)
        //
        //         holdContext = null
        //     }
        // })
    }

    private fun attachOutput(context: JSObject) {
        // val process = context.property("process").toObject()
        // val stdout = process.property("stdout").toObject()
        // stdout.property("write", object : JSFunction(stdout.context, "write") {
        //     @jsexport
        //     @Suppress("unused")
        //     fun write(string: String) {
        //         output.stdout(string)
        //     }
        // })
        //
        // val stderr = process.property("stderr").toObject()
        // stderr.property("write", object : JSFunction(stderr.context, "write") {
        //     @jsexport
        //     fun write(string: String) {
        //         output.stderr(string)
        //     }
        // })
    }

    @Suppress("unused")
    private fun onBeforeExit(exitCode: Int) {
        active = false
        context = WeakReference<JSContext>(null)
        done = true
        dispose()
        eventOnExit(exitCode)
    }

    private fun eventOnBooted(context: JSObject) {
        Log.i(TAG, "eventOnBooted")
        listeners.forEach { it.onNodeBooted(context) }
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

    private external fun start(args: Array<String>): Int

    private external fun dispose()

    private external fun setFs(fsPtr: Long)

    interface EventListener {
        fun onNodeBooted(context: JSObject)

        fun onNodeFinished(exitCode: Int)

        fun onNodeExited(exitCode: Int)

        fun onNodeError(error: JSException)
    }

    companion object {
        private const val TAG = "KNode"

        private val engineVersions = HashMap<String, String>()
        private val envs = HashMap<String, String>()
        //
        // @Suppress("unused")
        // const val kMediaAccessPermissionsNone = 0
        // @Suppress("unused")
        // const val kMediaAccessPermissionsRead = 1
        // @Suppress("unused")
        // const val kMediaAccessPermissionsWrite = 2
        // @Suppress("unused")
        // const val kMediaAccessPermissionsRW = 3

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
