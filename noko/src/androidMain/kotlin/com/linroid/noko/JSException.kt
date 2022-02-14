package com.linroid.noko

import com.linroid.noko.js.JSObject
import com.linroid.noko.js.JSValue

/**
 * @author linroid
 * @since 2019-10-19
 */
class JSException(val error: JSObject) : RuntimeException(error.toString()) {

  fun stack(): String {
    return error.get<JSValue>("stack").toString()
  }
}

