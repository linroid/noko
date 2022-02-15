package com.linroid.noko

import com.linroid.noko.type.JSObject
import com.linroid.noko.type.JSValue

/**
 * @author linroid
 * @since 2019-10-19
 */
class JSException(val error: JSObject) : RuntimeException(error.toString()) {
  fun stack(): String {
    return error.get<JSValue>("stack").toString()
  }
}

