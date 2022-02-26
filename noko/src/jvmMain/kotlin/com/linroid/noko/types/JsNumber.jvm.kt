package com.linroid.noko.types

import com.linroid.noko.Node
import com.linroid.noko.annotation.ForNative

actual open class JsNumber : JsValue {

  @ForNative
  private constructor(node: Node, ptr: Long) : super(node, ptr)

  constructor(node: Node, number: Number) : super(node, 0) {
    nativeNew(number.toDouble())
  }

  private external fun nativeNew(data: Double)
}