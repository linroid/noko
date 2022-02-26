package com.linroid.noko.types

import com.linroid.noko.Node

expect class JsString(node: Node, content: String) : JsValue {
  external fun nativeNew(content: String)
}
