package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node

class JsUndefined internal constructor(
  node: Node,
  pointer: NativePointer,
) : JsPrimitive<Unit>(node, pointer, Unit) {
  override fun toString(): String {
    return "undefined"
  }

  override fun toJson(): String? {
    return null
  }
}
