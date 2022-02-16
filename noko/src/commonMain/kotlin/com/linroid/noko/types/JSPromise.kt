package com.linroid.noko.types

import com.linroid.noko.JSException
import com.linroid.noko.Noko
import com.linroid.noko.annotation.ForNative
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException

suspend fun JSValue.awaitIfPromise(): JSValue {
  if (this is JSPromise) {
    return this.await()
  }
  return this
}

class JSPromise : JSObject {

  private var resolverPtr: Long = 0

  @ForNative
  private constructor(noko: Noko, nPtr: Long) : super(noko, nPtr)

  constructor(noko: Noko) : super(noko, 0) {
    nativeNew()
  }

  fun reject(error: String) {
    noko.post {
      nativeReject(JSError(noko, error))
    }
  }

  fun resolve(value: Any?) {
    noko.post {
      nativeResolve(from(noko, value))
    }
  }

  fun then(callback: (JSValue) -> Unit): JSPromise {
    val then = JSFunction(noko, "then") { _, argv ->
      check(argv.isNotEmpty()) { "then() should receive a JSValue argument" }
      val result: JSValue = argv[0]
      callback.invoke(result)
      return@JSFunction null
    }
    nativeThen(then)
    return this
  }

  fun catch(callback: (JSObject) -> Unit): JSPromise {
    val then = JSFunction(noko, "catch") { _, argv ->
      val result: JSValue = argv[0]
      check(result is JSObject) { "catch() should receive an JSObject parameter not ${result::class.simpleName}(${result.typeOf()})}: ${result.toJson()}" }
      callback.invoke(result)
      return@JSFunction null
    }
    nativeCatch(then)
    return this
  }

  suspend fun await(): JSValue {
    return suspendCancellableCoroutine { continuation ->
      if (!noko.post {
          then {
            continuation.resume(it)
          }.catch {
            continuation.resumeWithException(JSException(it))
          }
        }) {
        continuation.cancel()
      }
    }
  }

  private external fun nativeNew()
  private external fun nativeReject(error: JSError)
  private external fun nativeResolve(value: JSValue)

  private external fun nativeThen(callback: JSFunction)
  private external fun nativeCatch(callback: JSFunction)
}
