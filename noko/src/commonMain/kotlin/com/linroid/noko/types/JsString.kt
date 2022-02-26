package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node

expect class JsString : JsPrimitive<String> {

  constructor(node: Node, value: String)

  internal constructor(node: Node, pointer: NativePointer, value: String)
}
