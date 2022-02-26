package com.linroid.noko.types

import com.linroid.noko.Node
import com.linroid.noko.annotation.ForNative

class JsString : JsValue {

  @ForNative
  private constructor (node: Node, nPtr: Long) : super(node, nPtr)

  constructor(node: Node, content: String) : this(node, 0) {
    nativeNew(content)
  }

  external fun nativeNew(content: String)
}
