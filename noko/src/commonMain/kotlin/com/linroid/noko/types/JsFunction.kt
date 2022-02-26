package com.linroid.noko.types

expect open class JsFunction : JsObject {

  protected open fun onCall(receiver: JsValue, parameters: Array<out JsValue>): JsValue?

  fun call(receiver: JsValue, vararg parameters: Any?): JsValue

}
