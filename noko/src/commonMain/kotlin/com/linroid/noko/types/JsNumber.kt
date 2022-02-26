package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node

expect open class JsNumber<T : Number> : JsPrimitive<T> {
  internal constructor(
    node: Node,
    pointer: NativePointer,
    value: T,
  )

  constructor(
    node: Node,
    value: T,
  )
}
