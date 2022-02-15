package com.linroid.noko.types

import com.linroid.noko.Noko
import com.linroid.noko.annotation.ForNative

class JSError : JSObject {

  @ForNative
  private constructor (noko: Noko, nPtr: Long) : super(noko, nPtr)

  constructor(noko: Noko, message: String) : this(noko, 0) {
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
