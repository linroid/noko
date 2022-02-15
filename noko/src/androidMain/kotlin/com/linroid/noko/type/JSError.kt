package com.linroid.noko.type

import com.linroid.noko.annotation.NativeConstructor

/**
 * @author linroid
 * @since 2019-10-19
 */
class JSError : JSObject {

  @NativeConstructor
  private constructor (context: JSContext, reference: Long) : super(context, reference)

  constructor(context: JSContext, message: String) : this(context, 0) {
    nativeNew(message)
  }

  fun stack(): String {
    return get<JSValue>("stack").toString()
  }

  fun message(): String {
    return get<JSValue>("message").toString()
  }

  fun name(): String {
    return get<JSValue>("name").toString()
  }

  private external fun nativeNew(message: String);
}
