package com.linroid.noko.types

import com.linroid.noko.JsException
import com.linroid.noko.NativePointer
import com.linroid.noko.Node
import com.linroid.noko.NullNativePointer
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlin.coroutines.resume
import kotlin.coroutines.resumeWithException


actual class JsPromise : JsObject {

  internal actual constructor(node: Node, pointer: NativePointer) : super(node, pointer)

  actual constructor(node: Node) : super(node, nativeNew())

  actual fun reject(error: String) {
    node.post({
      nativeReject(JsError(node, error))
    }, false)
  }

  actual fun resolve(value: Any?) {
    node.post({
      nativeResolve(value)
    }, false)
  }

  actual fun then(callback: (Any?) -> Unit): JsPromise {
    val then = JsFunction(node, "then") { _, argv ->
      check(argv.isNotEmpty()) { "then() should receive a JsValue argument" }
      callback.invoke(argv[0])
      return@JsFunction null
    }
    nativeThen(then)
    return this
  }

  actual fun catch(callback: (JsObject) -> Unit): JsPromise {
    val then = JsFunction(node, "catch") { _, argv ->
      val result = argv[0]
      checkNotNull(result)
      check(result is JsObject) { "catch() should receive an JsObject parameter not ${result.javaClass.simpleName}" }
      callback.invoke(result)
      return@JsFunction null
    }
    nativeCatch(then)
    return this
  }

  actual suspend fun await(): Any? {
    return suspendCancellableCoroutine { continuation ->
      if (!node.post({
          then {
            continuation.resume(it)
          }.catch {
            continuation.resumeWithException(JsException(it.toString()))
          }
        }, false)) {
        continuation.cancel()
      }
    }
  }

  private external fun nativeReject(error: JsError)
  private external fun nativeResolve(value: Any?)

  private external fun nativeThen(callback: JsFunction)
  private external fun nativeCatch(callback: JsFunction)

  companion object {
    @JvmStatic
    private external fun nativeNew(): NativePointer
  }
}
