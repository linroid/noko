package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node

typealias Callable = (receiver: JsValue, parameters: Array<out Any?>) -> JsValue?

expect open class JsFunction : JsObject {

  internal constructor(node: Node, pointer: NativePointer)

  constructor(node: Node, name: String, callable: Callable? = null)

  protected open fun onCall(receiver: JsValue, parameters: Array<out Any?>): Any?

  fun call(receiver: JsValue, vararg parameters: Any?): Any?
}
