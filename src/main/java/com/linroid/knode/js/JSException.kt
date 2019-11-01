package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-19
 */
class JSException(val error: JSError) : RuntimeException() {
    fun stack(): String {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }
}
