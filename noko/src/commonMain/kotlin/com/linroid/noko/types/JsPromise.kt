package com.linroid.noko.types

import com.linroid.noko.Node

expect class JsPromise : JsObject {
  constructor(node: Node)

  fun reject(error: String)
  fun resolve(value: Any?)
  fun then(callback: (JsValue) -> Unit): JsPromise
  fun catch(callback: (JsObject) -> Unit): JsPromise
  suspend fun await(): JsValue
}

suspend fun JsValue.awaitIfPromise(): JsValue {
  if (this is JsPromise) {
    return this.await()
  }
  return this
}
