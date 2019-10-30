package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019/10/30
 */
class JSString(context: JSContext, reference: Long) : JSValue(context, reference) {

    override fun toInt(): Int {
        return toString().toInt()
    }

    override fun toBoolean(): Boolean {
        return toString().toBoolean()
    }

    override fun toDouble(): Double {
        return toString().toDouble()
    }
}
