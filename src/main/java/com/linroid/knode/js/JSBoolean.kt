package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-21
 */
class JSBoolean(context: JSContext, reference: Long, private val value: Boolean) : JSValue(context, reference) {

    override fun toBoolean(): Boolean {
        return value
    }

    override fun toString(): String = value.toString()

    override fun toInt(): Int {
        return if (value) 1 else 0
    }

    override fun toDouble(): Double {
        return if (value) 1.0 else 0.0
    }
}
