package com.linroid.noko.types

import com.linroid.noko.Node

actual open class JsPrimitive(node: Node, ptr: Long) : JsValue(node, ptr)
