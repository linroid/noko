package com.linroid.noko

import com.linroid.noko.types.JsObject
import com.linroid.noko.types.JsValue

class JsException(val error: JsObject) : RuntimeException(error.toString()) {
  fun stack(): String {
    return error.get<JsValue>("stack").toString()
  }
}

