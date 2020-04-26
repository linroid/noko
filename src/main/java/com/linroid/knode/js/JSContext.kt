package com.linroid.knode.js

import android.util.Log
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
  lateinit var node: KNode

  init {
    context = this
  }

  @Throws(JSException::class)
  fun eval(code: String, source: String = "", line: Int = 0): JSValue {
    return nativeEval(code, source, line)
  }

  @Throws(JSException::class)
  fun parseJson(json: String): JSValue {
    return nativeParseJson(json)
  }

  fun throwError(message: String): JSError {
    return nativeThrowError(message)
  }

  @Throws(JSException::class)
  @Deprecated("Not working")
  fun require(path: String): JSValue {
    return nativeRequire(path)
  }

  private external fun nativeEval(code: String, source: String, line: Int): JSValue
  private external fun nativeParseJson(json: String): JSValue
  private external fun nativeThrowError(message: String): JSError
  private external fun nativeRequire(path: String): JSObject
}
