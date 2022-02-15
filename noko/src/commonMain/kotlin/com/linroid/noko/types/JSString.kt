package com.linroid.noko.types

import com.linroid.noko.annotation.ForNative

class JSString : JSValue {
  @ForNative
  private constructor (context: JSContext, reference: Long) : super(context, reference)

  constructor(context: JSContext, content: String) : this(context, 0) {
    nativeNew(content)
  }

  external fun nativeNew(content: String)
}
