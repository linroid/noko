package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-23
 */
open class JSPrimitive(context: JSContext) : JSValue(context, 0) {
    override fun isPrimitive(): Boolean {
        return true
    }
}
