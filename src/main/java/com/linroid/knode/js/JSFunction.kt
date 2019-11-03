package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-19
 */
typealias Callable = (receiver: JSValue, parameters: Array<out JSValue>) -> JSValue?

open class JSFunction : JSObject {

    private val callable: Callable?
    private val name: String

    constructor(context: JSContext, reference: Long, name: String) : super(context, reference) {
        this.callable = null
        this.name = name
    }

    constructor(context: JSContext, name: String, callable: Callable? = null) : super(context, 0) {
        this.callable = callable
        this.name = name
        nativeNew()
    }

    protected open fun onCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue? {
        if (callable != null) {
            return callable.invoke(receiver, parameters)
        }
        return null
    }

    fun call(receiver: JSValue, vararg parameters: Any): JSValue? {
        val v8Parameters = Array(parameters.size) { from(context, parameters[it]) }
        return nativeCall(receiver, v8Parameters)
    }

    private external fun nativeCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue?
    private external fun nativeNew()
}
