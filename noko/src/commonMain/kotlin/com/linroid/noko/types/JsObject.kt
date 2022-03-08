package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node
import com.linroid.noko.observable.PropertiesObserver

/**
 * PropertyAttribute.
 */
data class PropertyDescriptor(
  val writeable: Boolean = true,
  val enumerable: Boolean = true,
  val configurable: Boolean = true,
  val value: Any? = null,
  val getter: JsFunction? = null,
  val setter: JsFunction? = null,
)

expect open class JsObject : JsValue {

  constructor(node: Node)

  internal constructor(node: Node, pointer: NativePointer)

  fun has(key: String): Boolean

  fun set(key: String, value: Any?)

  inline fun <reified T : Any> get(key: String): T?

  fun delete(key: String)

  fun keys(): Array<String>

  fun defineProperty(key: String, descriptor: PropertyDescriptor): Boolean

  /**
   * Watch [properties] to get value changed notifications
   */
  fun watch(observer: PropertiesObserver, vararg properties: String)
}
