package com.linroid.noko

import com.linroid.noko.type.JSObject
import com.linroid.noko.type.JSValue

class JSException(val error: JSObject) : RuntimeException(error.toString()) {
  fun stack(): String {
    return error.get<JSValue>("stack").toString()
  }
}

