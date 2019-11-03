package com.linroid.knode.js

import com.google.gson.JsonObject

/**
 * @author linroid
 * @since 2019-10-19
 */
open class JSObject : JSValue {

    protected constructor(context: JSContext?, reference: Long) : super(context, reference)

    constructor(context: JSContext) : this(context, 0) {
        nativeNew()
    }

    constructor(context: JSContext, data: JsonObject) : this(context, 0) {
        nativeNew()
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
        return when (T::class) {
            String::class -> value.toString()
            Int::class -> value.toInt()
            Float::class -> value.toDouble().toFloat()
            Double::class -> value.toDouble()
            JSFunction::class -> value as? JSFunction
            JSObject::class -> value as? JSObject
            JSValue::class -> value
//            else -> AppComponent.get().gson().fromJson(value.toJSON(), T::class.java)
            else -> TODO("not implemented")
        } as T
    }

    private external fun nativeKeys(): Array<String>
    private external fun nativeHas(key: String): Boolean
    private external fun nativeGet(key: String): JSValue
    private external fun nativeSet(key: String, value: JSValue?)
    private external fun nativeNew()
}
