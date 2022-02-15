package com.linroid.noko.types

import com.linroid.noko.annotation.ForNative

class JSNull @ForNative private constructor(context: JSContext, reference: Long) : JSPrimitive(context, reference) {

  override fun toJson(): String {
    return ""
  }
}
