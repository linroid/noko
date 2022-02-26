package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node
import com.linroid.noko.observable.PropertiesObserver
import kotlin.reflect.KClass

expect open class JsObject : JsValue {

  constructor(node: Node)

  internal constructor(node: Node, pointer: NativePointer)

  fun has(key: String): Boolean

  fun set(key: String, value: Any?)

  inline fun <reified T : Any> get(key: String): T

  inline fun <reified T : Any> opt(key: String): T?

  fun <T : Any> get(key: String, clazz: KClass<T>): T?

  fun delete(key: String)

  fun keys(): Array<String>

  /**
   * Watch [properties] to get value changed notifications
   */
  fun watch(vararg properties: String, observer: PropertiesObserver)
}
