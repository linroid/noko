package com.linroid.noko.type

import com.linroid.noko.JSException
import com.linroid.noko.annotation.NativeConstructor
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException

/**
 * @author linroid
 * @since 2019/11/1
 */

suspend fun JSValue.awaitIfPromise(): JSValue {
  if (this is JSPromise) {
    return this.await()
  }
  return this
}

class JSPromise : JSObject {

  private var resolverPtr: Long = 0

  @NativeConstructor
  private constructor(context: JSContext, reference: Long) : super(context, reference)

  constructor(context: JSContext) : super(context, 0) {
    nativeNew()
  }

  fun reject(error: String) {
    context.node.post {
      nativeReject(JSError(context, error))
    }
  }

  fun resolve(value: Any?) {
    context.node.post {
      nativeResolve(from(context, value))
    }
  }

  fun then(callback: (JSValue) -> Unit): JSPromise {
    val then = JSFunction(context, "then") { _, argv ->
      check(argv.isNotEmpty()) { "then() should receive a JSValue argument" }
      val result: JSValue = argv[0]
      callback.invoke(result)
      return@JSFunction null
    }
    nativeThen(then)
    return this
  }

  fun catch(callback: (JSObject) -> Unit): JSPromise {
    val then = JSFunction(context, "catch") { _, argv ->
      val result: JSValue = argv[0]
      check(result is JSObject) { "catch() should receive an JSObject parameter not ${result.javaClass.simpleName}(${result.typeOf()})}: ${result.toJson()}" }
      callback.invoke(result)
      return@JSFunction null
    }
    nativeCatch(then)
    return this
  }

  suspend fun await(): JSValue {
    return suspendCancellableCoroutine { continuation ->
      if (!context.node.post {
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
