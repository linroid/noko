package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-19
 */
class JSError : JSObject {

    @NativeConstructor
    private constructor (context: JSContext, reference: Long) : super(context, reference)

    constructor(context: JSContext, message: String) : this(context, 0) {
        nativeNew(message)
    }

    fun stack(): String {
        return get<JSString>("stack").toString()
    }

    fun message():String {
        return get<JSString>("message").toString()
    }

    private external fun nativeNew(message: String);
}
