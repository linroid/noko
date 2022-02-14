package com.linroid.noko.js

/**
 * @author linroid
 * @since 2019-10-21
 */
class JSBoolean @NativeConstructor private constructor(
  context: JSContext, reference: Long,
  private val data: Boolean,
) : JSPrimitive(context, reference) {

  fun get(): Boolean = data

  override fun toString(): String = data.toString()

  override fun toBoolean(): Boolean {
    return data
  }
}
