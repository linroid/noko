package com.linroid.knode.js

import android.util.Log
import com.google.gson.JsonObject
import com.google.gson.JsonSyntaxException
import java.lang.Exception
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
    }

    private fun addBinds() {
        if (javaClass == JSObject::class.java) {
            return
        }
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
                            Log.e(TAG, "Failed to invoke $name function", error)
                            context.throwError("A unexpected error occurs during call '$name' native function: ${error.targetException.message}")
                            return context.sharedUndefined
                        } catch (error: JsonSyntaxException) {
                            Log.e(TAG, "Failed to parse arguments for '$name' function", error)
                            context.throwError("Failed to parse arguments for '$name' native function")
                            return context.sharedUndefined
                        } catch (error: Exception) {
                            context.throwError("Unexpected error occurs when calling '$name' native function")
                            return context.sharedUndefined
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
        return opt<T>(key) ?: throw IllegalStateException("get $key from $this shouldn't return null")
    }

    fun delete(key: String) {
        nativeDelete(key)
    }

    fun keys(): Array<String> {
        return nativeKeys()
    }

    inline fun <reified T> opt(key: String): T? {
        val value = nativeGet(key)
        @Suppress("IMPLICIT_CAST_TO_ANY")
        return value.toType(T::class.java)
    }

    external fun nativeGet(key: String): JSValue
    private external fun nativeKeys(): Array<String>
    private external fun nativeHas(key: String): Boolean
    private external fun nativeSet(key: String, value: JSValue?)
    private external fun nativeDelete(key: String)
    private external fun nativeNew()

    companion object {
        private const val TAG = "JSObject"
    }
}
