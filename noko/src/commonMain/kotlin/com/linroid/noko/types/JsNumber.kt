package com.linroid.noko.types

import com.linroid.noko.Node
import com.linroid.noko.annotation.ForNative

open class JsNumber : JsValue {

  @ForNative
  private constructor(node: Node, nPtr: Long) : super(node, nPtr)

  constructor(node: Node, number: Number) : super(node, 0) {
    nativeNew(number.toDouble())
  }

  private external fun nativeNew(data: Double)
}
