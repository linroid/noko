package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node

actual class JsString : JsPrimitive<String> {

  internal actual constructor(
    node: Node,
    pointer: NativePointer,
    value: String,
  ) : super(node, pointer, value)

  actual constructor(node: Node, value: String) : super(node, value) {
    nativeNew(value)
  }

  private external fun nativeNew(value: String)
}
