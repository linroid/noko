package com.linroid.noko.types

import com.linroid.noko.Node
import com.linroid.noko.annotation.ForNative

class JsBoolean @ForNative private constructor(
  node: Node, nPtr: Long,
  private val data: Boolean,
) : JsPrimitive(node, nPtr) {

  fun get(): Boolean = data

  override fun toString(): String = data.toString()

  override fun toBoolean(): Boolean {
    return data
  }
}
