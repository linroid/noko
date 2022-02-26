package com.linroid.noko.types

import com.linroid.noko.Node
import com.linroid.noko.annotation.ForNative

class JsUndefined @ForNative private constructor(node: Node, ptr: Long) :
  JsPrimitive(node, ptr) {

  override fun toJson(): String {
    return ""
  }
}
