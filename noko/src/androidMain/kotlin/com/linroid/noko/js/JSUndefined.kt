package com.linroid.noko.js

/**
 * @author linroid
 * @since 2019-10-21
 */
class JSUndefined @NativeConstructor private constructor(context: JSContext, reference: Long) : JSPrimitive(context, reference) {

  override fun toJson(): String {
    return ""
  }
}
