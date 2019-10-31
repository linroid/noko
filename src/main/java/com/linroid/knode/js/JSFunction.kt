package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-19
 */
class JSFunction(context: JSContext, reference: Long) : JSObject(context, reference) {
    fun call(recv: JSValue, vararg parameters: JSValue): JSValue? {
        return nativeCall(context, parameters)
    }

    external fun nativeCall(recv: JSValue, parameters: Array<out JSValue>): JSValue?
}
