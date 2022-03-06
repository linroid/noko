package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node

expect class JsPromise(node: Node) : JsObject {

  internal constructor(node: Node, pointer: NativePointer)

  fun reject(error: String)

  fun resolve(value: Any?)

  fun then(callback: (Any?) -> Unit): JsPromise

  fun catch(callback: (JsObject) -> Unit): JsPromise

  suspend fun await(): Any?
}

suspend fun JsValue.awaitIfPromise(): Any? {
  if (this is JsPromise) {
    return this.await()
  }
  return this
}
