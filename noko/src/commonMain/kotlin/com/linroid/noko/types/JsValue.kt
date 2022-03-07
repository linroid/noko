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

  fun dispose()

  override fun equals(other: Any?): Boolean

  override fun hashCode(): Int
}
