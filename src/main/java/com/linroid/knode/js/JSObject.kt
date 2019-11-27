package com.linroid.knode.js

import android.util.Log
import com.google.gson.JsonObject
import java.lang.NullPointerException

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
                        val result = method.invoke(this@JSObject, *convertParameters(parameters, method.parameterTypes))
                        val jsRet = from(context, result)
                        Log.i("JSObject", "jsRet=$jsRet")
                        return jsRet
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
        Log.i("JSObject", "set $key=$value")
        nativeSet(key, from(context, value))
    }

    inline fun <reified T> get(key: String): T {
        return opt<T>(key) ?: throw IllegalStateException("get $key shouldn't return null")
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
}
