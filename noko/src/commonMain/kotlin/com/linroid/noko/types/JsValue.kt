package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node
import kotlin.reflect.KClass

expect open class JsValue internal constructor(node: Node, pointer: NativePointer) {

  internal val pointer: NativePointer

  protected val node: Node

  override fun toString(): String

  open fun toJson(): String?

  open fun toNumber(): Number

  open fun toBoolean(): Boolean

  fun typeOf(): String

  fun hasValue(): Boolean

  fun isPromise(): Boolean

  @Suppress("IMPLICIT_CAST_TO_ANY", "UNCHECKED_CAST")
  fun <T : Any> toType(type: KClass<T>): T?

  fun dispose()

  fun sameReference(other: JsValue): Boolean

  override fun equals(other: Any?): Boolean
  override fun hashCode(): Int
}
