package com.linroid.noko.types

import com.linroid.noko.Noko
import com.linroid.noko.annotation.ForNative
import java.lang.reflect.InvocationTargetException

typealias Callable = (receiver: JSValue, parameters: Array<out JSValue>) -> JSValue?

open class JSFunction : JSObject {

  private val callable: Callable?

 @ForNative
 private constructor(noko: Noko, ptr: Long) : super(noko, ptr) {
   this.callable = null
 }

  constructor(noko: Noko, name: String, callable: Callable? = null) : super(noko, 0) {
    this.callable = callable
    noko.checkThread()
    nativeNew(name)
  }

  protected open fun onCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue? {
    if (callable != null) {
      return try {
        callable.invoke(receiver, parameters)
      } catch (error: InvocationTargetException) {
        noko.throwError("An unexpected error occurred during call native function: ${error.getTargetException().message}")
      } catch (error: Exception) {
        // noko.throwError(error.message ?: "An error occurred when calling native function")
        throw error
      }
    }
    return null
  }

  fun call(receiver: JSValue, vararg parameters: Any?): JSValue {
    noko.checkThread()
    check(noko.ptr != 0L) { "node has been disposed" }
    val v8Parameters = Array(parameters.size) { from(noko, parameters[it]) }
    return nativeCall(receiver, v8Parameters)
  }

  private external fun nativeCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue
  private external fun nativeNew(name: String)
}
