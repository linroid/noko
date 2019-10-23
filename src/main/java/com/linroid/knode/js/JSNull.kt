package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-23
 */
class JSNull(context: JSContext) : JSPrimitive(context) {
    override fun isNull(): Boolean {
        return true
    }
}
