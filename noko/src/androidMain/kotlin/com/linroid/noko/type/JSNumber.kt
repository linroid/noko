package com.linroid.noko.type

import com.linroid.noko.annotation.NativeConstructor

/**
 * @author linroid
 * @since 2019-10-20
 */
open class JSNumber : JSValue {
  @NativeConstructor
  private constructor(context: JSContext, reference: Long) : super(context, reference)

  constructor(context: JSContext, number: Number) : super(context, 0) {
    nativeNew(number.toDouble())
  }

  private external fun nativeNew(data: Double)
}
