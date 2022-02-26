package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node
import com.linroid.noko.NullNativePointer
import com.linroid.noko.annotation.ForNative

actual open class JsNumber<T : Number> : JsPrimitive<T> {

  internal actual constructor(
    node: Node,
    pointer: NativePointer,
    value: T,
  ) : super(node, pointer, value)

  actual constructor(node: Node, value: T) : super(node, value) {
    nativeNew(value.toDouble())
  }

  private external fun nativeNew(data: Double)
}
