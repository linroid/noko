package com.linroid.noko.types

import com.linroid.noko.Noko
import com.linroid.noko.annotation.ForNative

class JSString : JSValue {

  @ForNative
  private constructor (noko: Noko, nPtr: Long) : super(noko, nPtr)

  constructor(noko: Noko, content: String) : this(noko, 0) {
    nativeNew(content)
  }

  external fun nativeNew(content: String)
}
