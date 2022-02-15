package com.linroid.noko.type

import com.linroid.noko.annotation.ForNative

class JSUndefined @ForNative private constructor(context: JSContext, reference: Long) : JSPrimitive(context, reference) {

  override fun toJson(): String {
    return ""
  }
}
