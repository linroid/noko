package com.linroid.knode.js

import com.google.gson.JsonObject

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
        javaClass.declaredMethods
            .filter { it.isAnnotationPresent(Bind::class.java) }
            .forEach { method ->
                method.isAccessible = true
                val bind = method.getAnnotation(Bind::class.java)!!
                val name = if (bind.name.isEmpty()) method.name else bind.name
                set(name, object : JSFunction(context, name) {
                    override fun onCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue? {
                        val result = method.invoke(this@JSObject, parameters)
                        return from(context, result)
                    }
                })
            }
    }

    fun has(key: String): Boolean {
        return nativeHas(key)
    }

    fun set(key: String, value: Any?) {
        nativeSet(key, from(context, value))
    }

    fun <T : JSValue> get(key: String): T {
        return nativeGet(key) as T
    }

    fun delete(key: String) {

    }

    fun keys(): Array<String> {
        return nativeKeys()
    }

    inline fun <reified T> opt(key: String): T? {
        if (!has(key)) {
            return null
        }
        val value = get<JSValue>(key)
        if (value is JSNull || value is JSUndefined) {
            return null
        }
        @Suppress("IMPLICIT_CAST_TO_ANY")
        return value.toType(T::class.java)
    }

    private external fun nativeKeys(): Array<String>
    private external fun nativeHas(key: String): Boolean
    private external fun nativeGet(key: String): JSValue
    private external fun nativeSet(key: String, value: JSValue?)
    private external fun nativeNew()
}
