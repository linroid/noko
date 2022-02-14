package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-23
 */
class JSNull @NativeConstructor private constructor(context: JSContext, reference: Long) : JSPrimitive(context, reference) {

  override fun toJson(): String {
    return ""
  }
}
