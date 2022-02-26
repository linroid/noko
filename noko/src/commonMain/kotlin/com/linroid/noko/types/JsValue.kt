package com.linroid.noko.types

import com.linroid.noko.Node
import kotlinx.serialization.encodeToString
import kotlinx.serialization.json.Json
import kotlinx.serialization.json.JsonElement
import kotlin.reflect.KClass

expect open class JsValue {
  protected val node: Node

  override fun toString(): String

  open fun toJson(): String

  open fun toNumber(): Number

  open fun toBoolean(): Boolean

  fun typeOf(): String

  fun hasValue(): Boolean

  fun isPromise(): Boolean

  @Suppress("IMPLICIT_CAST_TO_ANY", "UNCHECKED_CAST")
  fun <T : Any> toType(type: KClass<T>): T?

  fun dispose()

  override fun equals(other: Any?): Boolean

  fun sameReference(other: JsValue): Boolean

  override fun hashCode(): Int

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
