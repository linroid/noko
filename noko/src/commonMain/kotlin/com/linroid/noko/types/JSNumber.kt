package com.linroid.noko.types

import com.linroid.noko.Noko
import com.linroid.noko.annotation.ForNative

open class JSNumber : JSValue {

  @ForNative
  private constructor(noko: Noko, nPtr: Long) : super(noko, nPtr)

  constructor(noko: Noko, number: Number) : super(noko, 0) {
    nativeNew(number.toDouble())
  }

  private external fun nativeNew(data: Double)
}
