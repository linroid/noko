package com.linroid.noko.types

import com.linroid.noko.Noko
import com.linroid.noko.annotation.ForNative

class JSBoolean @ForNative private constructor(
  noko: Noko, nPtr: Long,
  private val data: Boolean,
) : JSPrimitive(noko, nPtr) {

  fun get(): Boolean = data

  override fun toString(): String = data.toString()

  override fun toBoolean(): Boolean {
    return data
  }
}
