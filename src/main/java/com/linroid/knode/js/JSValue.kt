package com.linroid.knode.js

import com.google.gson.JsonObject
import java.io.Closeable

/**
 * @author linroid
 * @since 2019-10-19
 */
open class JSValue(context: JSContext?, protected var reference: Long) : Closeable {
    @Suppress("LeakingThis")
    val context: JSContext = context ?: this as JSContext

    override fun close() {
        dispose(reference)
    }

    open fun isUndefined(): Boolean {
        return false
    }

    open fun isNull(): Boolean {
        return false
    }

    open fun isPrimitive(): Boolean {
        return false
    }

    override fun toString(): String {
        if (isUndefined()) {
            return null.toString()
        }
        return nativeToString()
    }

    open fun toJson(): String {
        return nativeToJson()
    }

    open fun toInt(): Int {
        return 0
    }

    open fun toDouble(): Double {
        return 0.0
    }

    open fun toBoolean(): Boolean {
        return true;
    }

    fun empty(): Boolean {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    fun isPromise(): Boolean {
        return false
    }

    private external fun nativeToJson(): String
    private external fun nativeToString(): String
    private external fun dispose(reference: Long)

    companion object {
        fun from(context: JSContext, value: Any?): JSValue {
            return when (value) {
                null -> JSNull(context)
                is JSValue -> value
                is String -> JSString(context, value)
                is Number -> JSNumber(context, value)
                is Iterator<*> -> JSArray(context, value)
                is JsonObject -> JSObject(context, value)
                else -> throw IllegalStateException("Not support ${value.javaClass}")
            }
        }
    }
}
