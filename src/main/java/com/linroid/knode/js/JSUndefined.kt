package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-21
 */
class JSUndefined(context: JSContext) : JSPrimitive(context) {

    override fun toJson(): String {
        return ""
    }
}
