package com.linroid.noko.types

import com.linroid.noko.annotation.ForNative

open class JSNumber : JSValue {
  @ForNative
  private constructor(context: JSContext, reference: Long) : super(context, reference)

  constructor(context: JSContext, number: Number) : super(context, 0) {
    nativeNew(number.toDouble())
  }

  private external fun nativeNew(data: Double)
}
