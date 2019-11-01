package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-19
 */
class JSContext(@Suppress("unused") private val runtimePtr: Long, reference: Long) : JSObject(null, reference) {

    fun eval(code: String, source: String = "", line: Int = 0): JSValue {
        return nativeEval(code, source, line)
    }

    fun setExceptionHandler(handler: Any) {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    private external fun nativeEval(code: String, source: String, line: Int): JSValue
}
