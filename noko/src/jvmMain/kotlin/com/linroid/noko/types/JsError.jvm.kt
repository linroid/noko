package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node
import com.linroid.noko.annotation.ForNative

actual class JsError : JsObject {

  @ForNative
  private constructor (node: Node, ptr: Long) : super(node, ptr)

  actual constructor(node: Node, message: String) : this(node, nativeNew(message))

  actual fun stack(): String {
    return get<JsValue>("stack").toString()
  }

  actual fun message(): String {
    return get<JsValue>("message").toString()
  }

  actual fun name(): String {
    return get<JsValue>("name").toString()
  }

  companion object {
    @JvmStatic
    private external fun nativeNew(message: String): NativePointer
  }
}
