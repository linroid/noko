package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-19
 */
typealias Callable = (receiver: JSValue, parameters: Array<out JSValue>) -> JSValue?

open class JSFunction : JSObject {

    private val callable: Callable?
    private val name: String

    constructor(context: JSContext, reference: Long) : super(context, reference) {
        callable = null
        name = ""
    }

    constructor(context: JSContext, name: String, callable: Callable) : super(context, 0) {
        this.callable = callable
        this.name = name
        nativeInit()
    }

    open fun onCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue? {
        if (callable != null) {
            return callable.invoke(receiver, parameters)
        }
        return null
    }

    fun call(receiver: JSValue, vararg parameters: JSValue): JSValue? {
        return nativeCall(receiver, parameters)
    }

    external fun nativeCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue?
    external fun nativeInit()
}
