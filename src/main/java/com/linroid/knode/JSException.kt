package com.linroid.knode

import com.linroid.knode.js.JSError

/**
 * @author linroid
 * @since 2019-10-19
 */
class JSException(val error: JSError) : RuntimeException(error.stack()) {

    fun stack(): String = message!!
}
