package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-19
 */
class JSError(context: JSContext, reference: Long) : JSObject(context, reference) {
    fun stack(): String {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    constructor(context: JSContext, message: String) : this(context, 0)
}
