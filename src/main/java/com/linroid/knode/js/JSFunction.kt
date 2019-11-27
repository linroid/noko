package com.linroid.knode.js

import android.util.Log

/**
 * @author linroid
 * @since 2019-10-19
 */
typealias Callable = (receiver: JSValue, parameters: Array<out JSValue>) -> JSValue?

open class JSFunction : JSObject {

    private val callable: Callable?

    @NativeConstructor
    private constructor(context: JSContext, reference: Long) : super(context, reference) {
        this.callable = null
    }

    constructor(context: JSContext, name: String, callable: Callable? = null) : super(context, 0) {
        this.callable = callable
        nativeNew(name)
        context.hold(this)
    }

    protected open fun onCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue? {
        if (callable != null) {
            val ret = try {
                callable.invoke(receiver, parameters)
            } catch (error: Exception) {
                error.printStackTrace()
                throw error
            }
            Log.i("JSFunction", ret.toString())

        }
        return null
    }

    fun call(receiver: JSValue, vararg parameters: Any): JSValue? {
        val v8Parameters = Array(parameters.size) { from(context, parameters[it]) }
        return nativeCall(receiver, v8Parameters)
    }

    private external fun nativeCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue?
    private external fun nativeNew(name: String)
}
