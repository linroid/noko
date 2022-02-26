package com.linroid.noko.types

import com.linroid.noko.Node
import com.linroid.noko.Platform
import com.linroid.noko.annotation.ForNative
import com.linroid.noko.ref.JSValueReference
import com.linroid.noko.runBlocking
import kotlinx.serialization.decodeFromString
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonArray
import kotlinx.serialization.json.JsonElement
import kotlinx.serialization.json.JsonObject
import okio.Closeable
import kotlin.jvm.JvmField
import kotlin.reflect.KClass

open class JsValue(
  protected val node: Node,
  @JvmField
  @Native internal val ptr: Long
) : Closeable {
  init {
    if (ptr != 0L) {
      // The object is just a wrapper for js object
      JSValueReference(this, node.cleaner)
    }
  }

  @ForNative
  private fun runtimePtr(): Long {
    return node.ptr
  }

  override fun toString(): String {
    if (Platform.isDebuggerConnected() && !node.isInNodeThread()) {
      return runBlocking {
        node.await { nativeToString() }
      }
    }
    node.checkThread()
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
    return this !is JsNull && this !is JsUndefined
  }

  fun isPromise(): Boolean {
    return false
  }

  @Suppress("IMPLICIT_CAST_TO_ANY", "UNCHECKED_CAST")
  fun <T : Any> toType(type: KClass<T>): T? {
    if (JsValue::class != type && !this.hasValue()) {
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
      // JsValue::class.isAssignableFrom(type) -> this
      // type.isArray -> {
      //   check(this is JsArray) { "$this is not an JsArray" }
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
    if (other is JsValue) {
      if (other.ptr == ptr) {
        return true
      }
      return nativeEquals(other)
    }
    return super.equals(other)
  }

  fun sameReference(other: JsValue): Boolean {
    return other.ptr == ptr
  }

  override fun hashCode(): Int {
    return ptr.hashCode()
  }

  private external fun nativeEquals(other: JsValue): Boolean
  private external fun nativeToJson(): String
  private external fun nativeToString(): String
  private external fun nativeToNumber(): Double
  private external fun nativeTypeOf(): String
  private external fun nativeDispose()

  companion object {

    fun from(node: Node, value: Any?): JsValue {
      return when (value) {
        null -> node.sharedNull
        is JsValue -> value
        is Boolean -> if (value) node.sharedTrue else node.sharedFalse
        is String -> JsString(node, value)
        is Number -> JsNumber(node, value)
        is Iterator<*> -> JsArray(node, value)
        is List<*> -> JsArray(node, value.iterator())
        is Array<*> -> JsArray(node, value.iterator())
        is JsonElement -> {
          node.parseJson(value.toString())
        }
        else -> {
          node.parseJson(Json.encodeToString(value))
        }
      }
    }

    // fun from(noko: Node, value: JsonElement): JsValue {
    //     return when {
    //         value.isJsonNull -> JsNull(noko)
    //         value.isJsonObject -> JsObject(noko, value.asJsonObject)
    //         value.isJsonArray -> JsArray(noko, value as Iterator<*>)
    //         value.isJsonPrimitive -> {
    //             value as JsonPrimitive
    //             when {
    //                 value.isBoolean -> JsBoolean(noko, value.asBoolean)
    //                 value.isNumber -> JsNumber(noko, value.asNumber)
    //                 value.isString -> JsString(noko, value.asString)
    //                 else -> throw IllegalStateException("Not support JsonPrimitive: ${value.javaClass}")
    //             }
    //         }
    //         else -> throw IllegalStateException("Not support Json type: ${value.javaClass}")
    //     }
    // }
  }
}
