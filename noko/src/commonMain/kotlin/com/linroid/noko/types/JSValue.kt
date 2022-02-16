package com.linroid.noko.types

import com.linroid.noko.Noko
import com.linroid.noko.Platform
import com.linroid.noko.ref.JSValueReference
import com.linroid.noko.runBlocking
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonArray
import kotlinx.serialization.json.JsonElement
import kotlinx.serialization.json.JsonObject
import java.io.Closeable
import java.lang.annotation.Native
import kotlin.reflect.KClass

open class JSValue(
  protected val noko: Noko,
  @Native internal val nPtr: Long
) : Closeable {
  init {
    if (nPtr != 0L) {
      // The object is just a wrapper for js object
      JSValueReference(this, noko.cleaner)
    }
  }

  /** For native access runtime ptr */
  private fun runtimePtr(): Long {
    return noko.nPtr
  }

  override fun toString(): String {
    if (Platform.isDebuggerConnected() && !noko.isInNodeThread()) {
      return runBlocking {
        noko.await { nativeToString() }
      }
    }
    noko.checkThread()
    return nativeToString()
  }

  open fun toJson(): String {
    return nativeToJson()
  }

  open fun toNumber(): Number {
    return nativeToNumber()
  }

  open fun toBoolean(): Boolean {
    return toNumber() != 0
  }

  fun typeOf(): String {
    return nativeTypeOf()
  }

  fun hasValue(): Boolean {
    return this !is JSNull && this !is JSUndefined
  }

  fun isPromise(): Boolean {
    return false
  }

  @Suppress("IMPLICIT_CAST_TO_ANY", "UNCHECKED_CAST")
  fun <T : Any> toType(type: KClass<T>): T? {
    if (JSValue::class != type && !this.hasValue()) {
      return null
    }
    val result = when {
      type == String::class -> this.toString()
      type == Any::class -> this
      type == Int::class -> toNumber().toInt()
      type == Boolean::class -> toBoolean()
      type == Long::class -> toNumber().toLong()
      type == Float::class -> toNumber().toFloat()
      type == Double::class -> toNumber().toDouble()
      type == JsonObject::class ||
          type == JsonElement::class ||
          type == JsonArray::class -> Json.decodeFromString(toJson())
      // JSValue::class.isAssignableFrom(type) -> this
      // type.isArray -> {
      //   check(this is JSArray) { "$this is not an JSArray" }
      //   this.map { it.toType(type.componentType as Class<out Any>) }.toTypedArray()
      // }
      else -> {
        val json = toJson()
        Json.decodeFromString(json)
      }
    }
    return result as T?
  }

  override fun close() {
    nativeDispose()
  }

  override fun equals(other: Any?): Boolean {
    if (other is JSValue) {
      if (other.nPtr == nPtr) {
        return true
      }
      return nativeEquals(other)
    }
    return super.equals(other)
  }

  fun sameReference(other: JSValue): Boolean {
    return other.nPtr == nPtr
  }

  override fun hashCode(): Int {
    return nPtr.hashCode()
  }

  private external fun nativeEquals(other: JSValue): Boolean
  private external fun nativeToJson(): String
  private external fun nativeToString(): String
  private external fun nativeToNumber(): Double
  private external fun nativeTypeOf(): String
  private external fun nativeDispose()

  companion object {

    fun from(noko: Noko, value: Any?): JSValue {
      return when (value) {
        null -> noko.sharedNull
        is JSValue -> value
        is Boolean -> if (value) noko.sharedTrue else noko.sharedFalse
        is String -> JSString(noko, value)
        is Number -> JSNumber(noko, value)
        is Iterator<*> -> JSArray(noko, value)
        is List<*> -> JSArray(noko, value.iterator())
        is Array<*> -> JSArray(noko, value.iterator())
        is JsonElement -> {
          noko.parseJson(value.toString())
        }
        else -> {
          noko.parseJson(Json.encodeToString(value))
        }
      }
    }

    // fun from(noko: Noko, value: JsonElement): JSValue {
    //     return when {
    //         value.isJsonNull -> JSNull(noko)
    //         value.isJsonObject -> JSObject(noko, value.asJsonObject)
    //         value.isJsonArray -> JSArray(noko, value as Iterator<*>)
    //         value.isJsonPrimitive -> {
    //             value as JsonPrimitive
    //             when {
    //                 value.isBoolean -> JSBoolean(noko, value.asBoolean)
    //                 value.isNumber -> JSNumber(noko, value.asNumber)
    //                 value.isString -> JSString(noko, value.asString)
    //                 else -> throw IllegalStateException("Not support JsonPrimitive: ${value.javaClass}")
    //             }
    //         }
    //         else -> throw IllegalStateException("Not support Json type: ${value.javaClass}")
    //     }
    // }
  }
}
