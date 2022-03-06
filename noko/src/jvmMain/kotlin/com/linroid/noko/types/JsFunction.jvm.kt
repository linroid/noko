package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node
import java.lang.reflect.InvocationTargetException

typealias Callable = (receiver: JsValue, parameters: Array<out Any?>) -> JsValue?

actual open class JsFunction : JsObject {

  private val callable: Callable?

  internal actual constructor(node: Node, pointer: NativePointer) : super(node, pointer) {
    this.callable = null
  }

  constructor(node: Node, name: String, callable: Callable? = null) : super(node, 0) {
    this.callable = callable
    node.checkThread()
    nativeNew(name)
  }

  protected actual open fun onCall(receiver: JsValue, parameters: Array<out Any?>): Any? {
    if (callable != null) {
      return try {
        callable.invoke(receiver, parameters)
      } catch (error: InvocationTargetException) {
        node.throwError("An unexpected error occurred during call native function: ${error.targetException.message}")
      } catch (error: Exception) {
        throw error
      }
    }
    return null
  }

  actual fun call(receiver: JsValue, vararg parameters: Any?): Any? {
    node.checkThread()
    check(node.pointer != 0L) { "node has already been disposed" }
    return nativeCall(receiver, parameters)
  }

  private external fun nativeCall(receiver: JsValue, parameters: Array<out Any?>): Any?
  private external fun nativeNew(name: String)
}
