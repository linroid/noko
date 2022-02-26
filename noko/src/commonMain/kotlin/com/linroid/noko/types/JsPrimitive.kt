package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node
import com.linroid.noko.NullNativePointer

abstract class JsPrimitive<T : Any> : JsValue {

  protected val value: T

  constructor(
    node: Node,
    pointer: NativePointer,
    value: T,
  ) : super(node, pointer) {
    this.value = value
  }

  internal constructor(node: Node, value: T) : super(node, NullNativePointer) {
    this.value = value
  }

  fun get(): T = value

  override fun toString(): String {
    return value.toString()
  }
}
