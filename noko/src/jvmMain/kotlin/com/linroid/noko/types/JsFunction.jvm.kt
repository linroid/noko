package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node
import com.linroid.noko.NullNativePointer
import java.lang.reflect.InvocationTargetException

actual open class JsFunction : JsObject {

  private val callable: Callable?
  private var name: String? = null

  internal actual constructor(node: Node, pointer: NativePointer) : super(node, pointer) {
    this.callable = null
  }

  /**
   * Won't create the associated v8 object until we set it into any v8 objects,
   * and a global reference will be created at that time to prevent this object from being recycled by jvm GC,
   * the global reference will be deleted when the v8 object gets cleaned
   */
  actual constructor(node: Node, name: String, callable: Callable?) : super(node, NullNativePointer) {
    this.callable = callable
    this.name = name
  }

  protected actual open fun onCall(receiver: JsValue, parameters: Array<out Any?>): Any? {
    if (callable != null) {
      return try {
        callable.invoke(receiver, parameters)
      } catch (error: InvocationTargetException) {
        node.throwError("An unexpected error occurred during calling native function: ${error.targetException.message}")
      } catch (error: Exception) {
        throw error
      }
    }
    return null
  }

  actual fun call(receiver: JsValue, vararg parameters: Any?): Any? {
    check(node.pointer != 0L) { "node has already been disposed" }
    return nativeCall(receiver, parameters)
  }

  private external fun nativeCall(receiver: JsValue, parameters: Array<out Any?>): Any?
}
