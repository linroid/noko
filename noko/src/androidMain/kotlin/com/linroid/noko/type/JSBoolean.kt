package com.linroid.noko.type

import com.linroid.noko.annotation.ForNative

class JSBoolean @ForNative private constructor(
  context: JSContext, reference: Long,
  private val data: Boolean,
) : JSPrimitive(context, reference) {

  fun get(): Boolean = data

  override fun toString(): String = data.toString()

  override fun toBoolean(): Boolean {
    return data
  }
}
