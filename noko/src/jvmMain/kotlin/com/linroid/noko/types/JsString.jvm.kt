package com.linroid.noko.types

import com.linroid.noko.Node
import com.linroid.noko.annotation.ForNative

actual class JsString : JsValue {

  @ForNative
  private constructor (node: Node, ptr: Long) : super(node, ptr)

  actual constructor(node: Node, content: String) : this(node, 0) {
    nativeNew(content)
  }

  actual external fun nativeNew(content: String)
}
