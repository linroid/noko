package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node

expect open class JsFunction internal constructor(
  node: Node,
  pointer: NativePointer,
) : JsObject {

  protected open fun onCall(receiver: JsValue, parameters: Array<out Any?>): Any?

  fun call(receiver: JsValue, vararg parameters: Any?): Any?
}
