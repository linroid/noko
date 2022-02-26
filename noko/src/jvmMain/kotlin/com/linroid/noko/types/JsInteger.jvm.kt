package com.linroid.noko.types

import com.linroid.noko.Node

actual class JsInteger actual constructor(node: Node, value: Int) : JsNumber(node, value)
