package com.linroid.knode.js

import com.linroid.knode.JSException
import com.linroid.knode.KNode
import java.lang.annotation.Native

/**
 * @author linroid
 * @since 2019-10-19
 */
class JSContext @NativeConstructor private constructor(
    @Native internal val runtimePtr: Long,
    reference: Long
) : JSObject(null, reference) {

    internal lateinit var sharedNull: JSNull
    internal lateinit var sharedUndefined: JSUndefined
    internal lateinit var sharedTrue: JSBoolean
    internal lateinit var sharedFalse: JSBoolean
    internal lateinit var thread: Thread
    lateinit var node: KNode

    init {
        context = this
    }

    private val references = HashSet<JSValue>()

    @Throws(JSException::class)
    fun eval(code: String, source: String = "", line: Int = 0): JSValue {
        return nativeEval(code, source, line)
    }

    @Throws(JSException::class)
    fun parseJson(json: String): JSValue {
        return nativeParseJson(json)
    }

    fun hold(obj: JSValue) {
        references.add(obj)
    }

    fun throwError(message: String): JSError {
        return nativeThrowError(message)
    }

    private external fun nativeEval(code: String, source: String, line: Int): JSValue
    private external fun nativeParseJson(json: String): JSValue
    private external fun nativeThrowError(message: String): JSError
}
