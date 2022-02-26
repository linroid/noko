package com.linroid.noko.types

expect class JsUndefined : JsPrimitive {
  override fun toJson(): String
}
