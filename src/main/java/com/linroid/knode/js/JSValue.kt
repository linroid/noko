package com.linroid.knode.js

import java.io.Closeable

/**
 * @author linroid
 * @since 2019-10-19
 */
open class JSValue(protected val context: JSContext?, protected val reference: Long) : Closeable {

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

    external fun toJSON(): String

    private external fun nativeToString(): String

    private external fun dispose(reference: Long)
}
