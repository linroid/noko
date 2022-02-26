package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node
import com.linroid.noko.NullNativePointer

actual class JsInteger : JsNumber<Int> {

  actual constructor(node: Node, value: Int) : super(node, NullNativePointer, value) {
    nativeNew(value)
  }

  internal actual constructor(
    node: Node,
    pointer: NativePointer,
    value: Int,
  ) : super(node, pointer, value)

  private external fun nativeNew(data: Int)
}
