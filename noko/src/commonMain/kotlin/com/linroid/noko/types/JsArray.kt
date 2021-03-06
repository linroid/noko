package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node

expect class JsArray : JsObject, MutableList<Any?> {
  internal constructor(node: Node, pointer: NativePointer)
  constructor(node: Node, data: Iterable<*> = emptyList<Any>())
}
