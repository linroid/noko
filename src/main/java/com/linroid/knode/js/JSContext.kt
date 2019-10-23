package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-19
 */
class JSContext(@Suppress("unused") private val runtimePtr: Long, reference: Long) : JSObject(null, reference) {

    fun eval(code: String, source: String = "", line: Int = 0): Any {
        return nativeEval(code, source, line)
    }

    fun newObject(ob: Any) {

    }

    private external fun nativeEval(code: String, source: String, line: Int): Any
}
