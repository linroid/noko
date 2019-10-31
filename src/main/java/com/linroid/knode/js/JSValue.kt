package com.linroid.knode.js

import java.io.Closeable

/**
 * @author linroid
 * @since 2019-10-19
 */
@Suppress("LeakingThis")
open class JSValue(context: JSContext?, protected var reference: Long) : Closeable {
    protected val context: JSContext = context ?: this as JSContext

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

    private external fun nativeToJson(): String
    private external fun nativeToString(): String

    private external fun dispose(reference: Long)
}
