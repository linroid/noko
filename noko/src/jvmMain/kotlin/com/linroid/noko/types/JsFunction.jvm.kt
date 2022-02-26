package com.linroid.noko.types

import com.linroid.noko.Node
import com.linroid.noko.annotation.ForNative
import java.lang.reflect.InvocationTargetException

typealias Callable = (receiver: JsValue, parameters: Array<out JsValue>) -> JsValue?

actual open class JsFunction : JsObject {

  private val callable: Callable?

 @ForNative
 private constructor(node: Node, ptr: Long) : super(node, ptr) {
   this.callable = null
 }

  constructor(node: Node, name: String, callable: Callable? = null) : super(node, 0) {
    this.callable = callable
    node.checkThread()
    nativeNew(name)
  }

  protected actual open fun onCall(receiver: JsValue, parameters: Array<out JsValue>): JsValue? {
    if (callable != null) {
      return try {
        callable.invoke(receiver, parameters)
      } catch (error: InvocationTargetException) {
        node.throwError("An unexpected error occurred during call native function: ${error.getTargetException().message}")
      } catch (error: Exception) {
        // noko.throwError(error.message ?: "An error occurred when calling native function")
        throw error
      }
    }
    return null
  }

  actual fun call(receiver: JsValue, vararg parameters: Any?): JsValue {
    node.checkThread()
    check(node.ptr != 0L) { "node has been disposed" }
    val v8Parameters = Array(parameters.size) { from(node, parameters[it]) }
    return nativeCall(receiver, v8Parameters)
  }

  private external fun nativeCall(receiver: JsValue, parameters: Array<out JsValue>): JsValue
  private external fun nativeNew(name: String)
}
