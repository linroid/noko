package com.linroid.noko.types

import com.linroid.noko.JsException
import com.linroid.noko.NativePointer
import com.linroid.noko.Node
import com.linroid.noko.NullNativePointer
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException


actual class JsPromise : JsObject {

  private var resolverPointer: NativePointer = 0

  internal actual constructor(node: Node, pointer: NativePointer) : super(node, pointer)

  actual constructor(node: Node) : super(node, NullNativePointer) {
    nativeNew()
  }

  actual fun reject(error: String) {
    node.post {
      nativeReject(JsError(node, error))
    }
  }

  actual fun resolve(value: Any?) {
    node.post {
      nativeResolve(from(node, value))
    }
  }

  actual fun then(callback: (JsValue) -> Unit): JsPromise {
    val then = JsFunction(node, "then") { _, argv ->
      check(argv.isNotEmpty()) { "then() should receive a JsValue argument" }
      val result: JsValue = argv[0]
      callback.invoke(result)
      return@JsFunction null
    }
    nativeThen(then)
    return this
  }

  actual fun catch(callback: (JsObject) -> Unit): JsPromise {
    val then = JsFunction(node, "catch") { _, argv ->
      val result: JsValue = argv[0]
      check(result is JsObject) { "catch() should receive an JsObject parameter not ${result::class.simpleName}(${result.typeOf()})}: ${result.toJson()}" }
      callback.invoke(result)
      return@JsFunction null
    }
    nativeCatch(then)
    return this
  }

  actual suspend fun await(): JsValue {
    return suspendCancellableCoroutine { continuation ->
      if (!node.post {
          then {
            continuation.resume(it)
          }.catch {
            continuation.resumeWithException(JsException(it))
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
