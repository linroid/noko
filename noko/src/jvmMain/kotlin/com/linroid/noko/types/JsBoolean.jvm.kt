package com.linroid.noko.types

import com.linroid.noko.Node
import com.linroid.noko.annotation.ForNative

class JsBoolean @ForNative private constructor(
  node: Node, ptr: Long,
  private val data: Boolean,
) : JsPrimitive(node, ptr) {

  fun get(): Boolean = data

  override fun toString(): String = data.toString()

  override fun toBoolean(): Boolean {
    return data
  }
}
