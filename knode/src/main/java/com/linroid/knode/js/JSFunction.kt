package com.linroid.knode.js

import java.lang.reflect.InvocationTargetException

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
    context.node.checkThread()
    nativeNew(name)
  }

  protected open fun onCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue? {
    if (callable != null) {
      return try {
        callable.invoke(receiver, parameters)
      } catch (error: InvocationTargetException) {
        context.throwError("A unexpected error occurs during call native function: ${error.targetException.message}")
      } catch (error: Exception) {
        // context.throwError(error.message ?: "An error occurred when calling native function")
        throw error
      }
    }
    return null
  }

  fun call(receiver: JSValue, vararg parameters: Any?): JSValue {
    context.node.checkThread()
    check(context.runtimePtr != 0L) { "node has been disposed" }
    val v8Parameters = Array(parameters.size) { from(context, parameters[it]) }
    return nativeCall(receiver, v8Parameters)
  }

  private external fun nativeCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue
  private external fun nativeNew(name: String)

  companion object {
    private const val TAG = "JSFunction"
  }
}
