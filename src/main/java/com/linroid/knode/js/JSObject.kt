package com.linroid.knode.js

import android.util.Log
import com.google.gson.JsonObject
import com.linroid.knode.BuildConfig
import java.lang.reflect.InvocationTargetException

/**
 * @author linroid
 * @since 2019-10-19
 */
open class JSObject : JSValue {

    @NativeConstructor
    protected constructor(context: JSContext?, reference: Long) : super(context, reference)

    constructor(context: JSContext, data: JsonObject? = null) : super(context, 0) {
        nativeNew()
        data?.entrySet()?.forEach {
            set(it.key, from(context, it.value))
        }
        addBinds()

        if (BuildConfig.DEBUG) {
            check(Thread.currentThread() == context.thread) {
                "Couldn't operate node object non origin thread: ${Thread.currentThread()}"
            }
        }
    }

    private fun addBinds() {
        if (javaClass == JSObject::class.java) {
            return
        }
        val className = javaClass.simpleName
        javaClass.methods
            .filter { it.isAnnotationPresent(BindJS::class.java) }
            .forEach { method ->
                method.isAccessible = true
                val bind = method.getAnnotation(BindJS::class.java)!!
                val name = if (bind.name.isEmpty()) method.name else bind.name
                set(name, object : JSFunction(context, name) {
                    override fun onCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue? {
                        val result = try {
                            method.invoke(this@JSObject, *convertParameters(parameters, method.parameterTypes))
                        } catch (error: InvocationTargetException) {
                            Log.e(TAG, "Exception when calling '${className}#$name()'", error.targetException)
                            context.throwError("Unexpected error occurred during call '${className}#$name()': ${error.targetException.message}")
                        } catch (error: Exception) {
                            Log.e(TAG, "Exception when calling '${className}#$name()'", error)
                            context.throwError("Unexpected error occurred during call '${className}#$name()': ${error.message}")
                        }
                        return from(context, result)
                    }
                })
            }
    }

    private fun convertParameters(parameters: Array<out JSValue>, parameterTypes: Array<Class<*>>): Array<Any?> {
        val argc = parameterTypes.size
        return Array(argc) { i ->
            val value = parameters[i]
            val type = parameterTypes[i]
            value.toType(type)
        }
    }

    fun has(key: String): Boolean {
        return nativeHas(key)
    }

    fun set(key: String, value: Any?) {
        // Log.d(TAG, "set $key @${Thread.currentThread().name}")
        // Thread.dumpStack()
        nativeSet(key, from(context, value))
    }

    inline fun <reified T> get(key: String): T {
        return get(key, T::class.java) ?: throw IllegalStateException("get $key from $this shouldn't return null")
    }

    inline fun <reified T> opt(key: String): T? {
        return get(key, T::class.java)
    }

    fun <T> get(key: String, clazz: Class<T>): T? {
        if (BuildConfig.DEBUG) {
            check(Thread.currentThread() == context.thread) {
                "Couldn't operate node object non origin thread: ${Thread.currentThread()}"
            }
        }
        val value = nativeGet(key)
        return value.toType(clazz)
    }

    fun delete(key: String) {
        nativeDelete(key)
    }

    fun keys(): Array<String> {
        return nativeKeys()
    }

    private external fun nativeGet(key: String): JSValue
    private external fun nativeKeys(): Array<String>
    private external fun nativeHas(key: String): Boolean
    private external fun nativeSet(key: String, value: JSValue?)
    private external fun nativeDelete(key: String)
    private external fun nativeNew()

    companion object {
        private const val TAG = "JSObject"
    }
}
