package com.linroid.knode.js

import com.google.gson.JsonElement
import java.lang.reflect.Method

/**
 * @author linroid
 * @since 2019-10-19
 */
open class JSObject(context: JSContext?, reference: Long) : JSValue(context, reference) {

    constructor(context: JSContext?) : this(context, 0) {
    }

    fun has(key: String): Boolean {
        return nativeHas(key)
    }

    fun set(key: String, value: JSValue?) {
        nativeSet(key, value)
    }

    fun set(key: String, value: String) {
        set(key, JSString(context, value))
    }

    fun set(key: String, value: Int) {
        set(key, JSNumber(context, value))
    }

    fun set(key: String, json: JsonElement) {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
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
}
