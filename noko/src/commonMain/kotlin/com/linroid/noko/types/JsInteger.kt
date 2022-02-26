package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node

expect class JsInteger : JsNumber<Int> {

  constructor(node: Node, value: Int)

  internal constructor(node: Node, pointer: NativePointer, value: Int)
}
