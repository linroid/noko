package com.linroid.knode.js

import com.linroid.knode.JSException

/**
 * @author linroid
 * @since 2019-10-19
 */
class JSContext(@Suppress("unused") private val runtimePtr: Long, reference: Long) : JSObject(null, reference) {
    private val references = HashSet<JSValue>()

    @Throws(JSException::class)
    fun eval(code: String, source: String = "", line: Int = 0): JSValue {
        return nativeEval(code, source, line)
    }

    @Throws(JSException::class)
    fun parseJson(json: String): JSValue {
        return nativeParseJson(json)
    }

    fun setExceptionHandler(handler: Any) {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    fun hold(obj: JSValue) {
        references.add(obj)
    }

    private external fun nativeEval(code: String, source: String, line: Int): JSValue
    private external fun nativeParseJson(json: String): JSValue
}
