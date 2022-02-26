package com.linroid.noko.types

import com.linroid.noko.JSException
import com.linroid.noko.Node
import com.linroid.noko.annotation.ForNative
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException

suspend fun JsValue.awaitIfPromise(): JsValue {
  if (this is JsPromise) {
    return this.await()
  }
  return this
}

class JsPromise : JsObject {

  private var resolverPtr: Long = 0

  @ForNative
  private constructor(node: Node, ptr: Long) : super(node, ptr)

  constructor(node: Node) : super(node, 0) {
    nativeNew()
  }

  fun reject(error: String) {
    node.post {
      nativeReject(JsError(node, error))
    }
  }

  fun resolve(value: Any?) {
    node.post {
      nativeResolve(from(node, value))
    }
  }

  fun then(callback: (JsValue) -> Unit): JsPromise {
    val then = JsFunction(node, "then") { _, argv ->
      check(argv.isNotEmpty()) { "then() should receive a JsValue argument" }
      val result: JsValue = argv[0]
      callback.invoke(result)
      return@JsFunction null
    }
    nativeThen(then)
    return this
  }

  fun catch(callback: (JsObject) -> Unit): JsPromise {
    val then = JsFunction(node, "catch") { _, argv ->
      val result: JsValue = argv[0]
      check(result is JsObject) { "catch() should receive an JsObject parameter not ${result::class.simpleName}(${result.typeOf()})}: ${result.toJson()}" }
      callback.invoke(result)
      return@JsFunction null
    }
    nativeCatch(then)
    return this
  }

  suspend fun await(): JsValue {
    return suspendCancellableCoroutine { continuation ->
      if (!node.post {
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
  private external fun nativeReject(error: JsError)
  private external fun nativeResolve(value: JsValue)

  private external fun nativeThen(callback: JsFunction)
  private external fun nativeCatch(callback: JsFunction)
}
