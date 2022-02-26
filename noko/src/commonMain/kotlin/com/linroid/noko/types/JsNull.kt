package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node

class JsNull internal constructor(
  node: Node,
  pointer: NativePointer,
) : JsObject(node, pointer) {
  override fun toString(): String {
    return "null"
  }

  override fun toJson(): String {
    return "null"
  }
}
