package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node
import com.linroid.noko.annotation.ForNative

actual class JsError : JsObject {

  @ForNative
  private constructor (node: Node, pointer: Long) : super(node, pointer)

  actual constructor(node: Node, message: String) : this(node, nativeNew(message))

  actual fun stack(): String {
    return get<JsValue>("stack").toString()
  }

  actual fun message(): String {
    return get<String>("message").toString()
  }

  actual fun name(): String {
    return get<String>("name").toString()
  }

  companion object {
    @JvmStatic
    private external fun nativeNew(message: String): NativePointer
  }
}
