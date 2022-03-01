package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node

actual open class JsNumber<T : Number> : JsPrimitive<T> {

  internal actual constructor(
    node: Node,
    pointer: NativePointer,
    value: T,
  ) : super(node, pointer, value)

  actual constructor(node: Node, value: T) : this(
    node,
    nativeNew(node.pointer, value.toDouble()),
    value
  )

  companion object {
    @JvmStatic
    private external fun nativeNew(nodePointer: NativePointer, data: Double): NativePointer
  }
}
