package com.linroid.noko.types

import com.linroid.noko.Noko
import com.linroid.noko.annotation.ForNative

class JSUndefined @ForNative private constructor(noko: Noko, nPtr: Long) :
  JSPrimitive(noko, nPtr) {

  override fun toJson(): String {
    return ""
  }
}
