package com.linroid.noko.types

import com.linroid.noko.*
import com.linroid.noko.annotation.ForNative
import com.linroid.noko.ref.JSValueReference
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonArray
import kotlinx.serialization.json.JsonElement
import kotlinx.serialization.json.JsonObject

actual open class JsValue actual constructor(
  protected actual val node: Node,
  @JvmField
  internal actual val pointer: NativePointer,
) {
  init {
    if (pointer != 0L) {
      // The object is just a wrapper for js object
      JSValueReference(this, node.cleaner)
    }
  }

  @ForNative
  private fun nodePointer(): Long {
    return node.pointer
  }

  actual override fun toString(): String {
    if (Platform.isDebuggerConnected() && !node.isInEventLoop()) {
      return runBlocking {
        node.await { nativeToString() }
      }
    }
    node.checkThread()
    return nativeToString()
  }

  actual open fun toJson(): String? {
    return nativeToJson()
  }

  actual open fun toNumber(): Number {
    return nativeToNumber()
  }

  actual open fun toBoolean(): Boolean {
    return toNumber() != 0
  }

  actual fun typeOf(): String {
    if (pointer == NullNativePointer) {
      return "undefined"
    }
    return nativeTypeOf()
  }

  actual fun hasValue(): Boolean {
    return this !is JsUndefined && pointer != NullNativePointer
  }

  actual fun isPromise(): Boolean {
    return false
  }

  actual inline fun <reified T : Any> toType(): T? {
    return toType(T::class.java)
  }

  @Suppress("IMPLICIT_CAST_TO_ANY", "UNCHECKED_CAST")
  fun <T : Any> toType(type: Class<T>): T? {
    if (JsValue::class != type && !this.hasValue()) {
      return null
    }
    if (JsValue::class.java.isAssignableFrom(type)) {
      return this as T
    }
    if (type.isArray) {
      check(this is JsArray) { "$this is not JsArray" }
      return this as T
    }
    val result = when (type) {
      String::class.java -> this.toString()
      Any::class.java -> this
      java.lang.Integer::class.java -> toNumber().toInt()
      java.lang.Boolean::class.java -> toBoolean()
      java.lang.Long::class.java -> toNumber().toLong()
      java.lang.Float::class.java -> toNumber().toFloat()
      java.lang.Double::class.java -> toNumber().toDouble()
      JsonObject::class.java,
      JsonElement::class.java,
      JsonArray::class.java -> Json.decodeFromString(toJson() ?: return null)
      else -> {
        val json = toJson() ?: return null
        Json.decodeFromString(json)
      }
    }
    return result as T?
  }

  actual fun dispose() {
    nativeDispose()
  }

  actual override fun equals(other: Any?): Boolean {
    if (other is JsValue) {
      if (other.pointer == pointer) {
        return true
      }
      return nativeEquals(other)
    }
    return super.equals(other)
  }

  actual fun sameReference(other: JsValue): Boolean {
    return other.pointer == pointer
  }

  actual override fun hashCode(): Int {
    return pointer.hashCode()
  }

  private external fun nativeEquals(other: JsValue): Boolean
  private external fun nativeToJson(): String?
  private external fun nativeToString(): String
  private external fun nativeToNumber(): Double
  private external fun nativeTypeOf(): String
  private external fun nativeDispose()

}
