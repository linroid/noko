package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node

class JsBoolean internal constructor(
  node: Node,
  pointer: NativePointer,
  value: Boolean
) : JsPrimitive<Boolean>(node, pointer, value) {
  override fun toJson(): String {
    return value.toString()
  }
}

