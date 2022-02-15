package com.linroid.noko.type

import com.linroid.noko.annotation.NativeConstructor

/**
 * @author linroid
 * @since 2019/10/30
 */
class JSString : JSValue {
  @NativeConstructor
  private constructor (context: JSContext, reference: Long) : super(context, reference)

  constructor(context: JSContext, content: String) : this(context, 0) {
    nativeNew(content)
  }

  external fun nativeNew(content: String)
}
