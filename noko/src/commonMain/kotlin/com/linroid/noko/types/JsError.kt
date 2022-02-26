package com.linroid.noko.types

import com.linroid.noko.Node

expect class JsError(node: Node, message: String) : JsObject {
  fun stack(): String
  fun message(): String
  fun name(): String
}
