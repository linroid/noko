package com.linroid.noko.types

import com.linroid.noko.Node
import com.linroid.noko.annotation.ForNative

class JsError : JsObject {

  @ForNative
  private constructor (node: Node, ptr: Long) : super(node, ptr)

  constructor(node: Node, message: String) : this(node, 0) {
    nativeNew(message)
  }

  fun stack(): String {
    return get<JsValue>("stack").toString()
  }

  fun message(): String {
    return get<JsValue>("message").toString()
  }

  fun name(): String {
    return get<JsValue>("name").toString()
  }

  private external fun nativeNew(message: String)
}
