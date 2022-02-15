package com.linroid.noko.types

import com.google.gson.JsonArray
import com.google.gson.JsonElement
import com.google.gson.JsonObject
import com.google.gson.JsonSyntaxException
import com.linroid.noko.ref.JSValueReference
import com.linroid.noko.Noko
import com.linroid.noko.Platform
import kotlinx.coroutines.runBlocking
import java.io.Closeable
import java.lang.annotation.Native
import java.net.URI

open class JSValue(context: JSContext? = null, @Native internal val reference: Long) : Closeable {

  @Suppress("LeakingThis")
  lateinit var context: JSContext

  init {
    if (context != null) {
      this.context = context
      if (reference != 0L) {
        // The object is just a wrapper for js object
        JSValueReference(this, context.cleaner)
      }
    }
    //TODO: Monitor js object is collected if the object is created by java side
  }

  /** For native access runtime ptr */
  private fun runtimePtr(): Long {
    return context.runtimePtr
  }

  override fun toString(): String {
    if (Platform.isDebuggerConnected() && !context.node.isInNodeThread()) {
      return runBlocking { context.node.await { nativeToString() } }
    }
    context.node.checkThread()
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
  fun <T> toType(type: Class<T>): T? {
    if (JSValue::class.java != type && !this.hasValue()) {
      return null
    }
    val result = when {
      type == String::class.java -> this.toString()
      type == Any::class.java -> this
      type == Int::class.java -> toNumber().toInt()
      type == Boolean::class.java -> toBoolean()
      type == Long::class.java -> toNumber().toLong()
      type == Float::class.java -> toNumber().toFloat()
      type == Double::class.java -> toNumber().toDouble()
      type == URI::class.java -> try {
        URI.create(toString())
      } catch (error: Exception) {
        null
      }
      type == JsonObject::class.java ||
          type == JsonElement::class.java ||
          type == JsonArray::class.java -> Noko.gson.fromJson(toJson(), JsonElement::class.java)
      JSValue::class.java.isAssignableFrom(type) -> this
      type.isArray -> {
        check(this is JSArray) { "$this is not an JSArray" }
        this.map { it.toType(type.componentType as Class<out Any>) }.toTypedArray()
      }
      else -> {
        val json = toJson()
        try {
          Noko.gson.fromJson(json, type)
        } catch (error: JsonSyntaxException) {
          throw IllegalArgumentException("Unable to parse the json as $type: $json", error)
        }
      }
    }
    return result as T?
  }

  override fun close() {
    nativeDispose()
  }

  override fun equals(other: Any?): Boolean {
    if (other is JSValue) {
      if (other.reference == reference) {
        return true
      }
      return nativeEquals(other)
    }
    return super.equals(other)
  }

  fun sameReference(other: JSValue): Boolean {
    return other.reference == reference
  }

  override fun hashCode(): Int {
    return reference.hashCode()
  }

  private external fun nativeEquals(other: JSValue): Boolean
  private external fun nativeToJson(): String
  private external fun nativeToString(): String
  private external fun nativeToNumber(): Double
  private external fun nativeTypeOf(): String
  private external fun nativeDispose()

  companion object {

    fun from(context: JSContext, value: Any?): JSValue {
      return when (value) {
        null -> context.sharedNull
        is JSValue -> value
        is Boolean -> if (value) context.sharedTrue else context.sharedFalse
        is String -> JSString(context, value)
        is Number -> JSNumber(context, value)
        is Iterator<*> -> JSArray(context, value)
        is List<*> -> JSArray(context, value.iterator())
        is Array<*> -> JSArray(context, value.iterator())
        is JsonElement -> {
          context.parseJson(value.toString())
        }
        else -> {
          context.parseJson(Noko.gson.toJson(value))
        }
      }
    }

    // fun from(context: JSContext, value: JsonElement): JSValue {
    //     return when {
    //         value.isJsonNull -> JSNull(context)
    //         value.isJsonObject -> JSObject(context, value.asJsonObject)
    //         value.isJsonArray -> JSArray(context, value as Iterator<*>)
    //         value.isJsonPrimitive -> {
    //             value as JsonPrimitive
    //             when {
    //                 value.isBoolean -> JSBoolean(context, value.asBoolean)
    //                 value.isNumber -> JSNumber(context, value.asNumber)
    //                 value.isString -> JSString(context, value.asString)
    //                 else -> throw IllegalStateException("Not support JsonPrimitive: ${value.javaClass}")
    //             }
    //         }
    //         else -> throw IllegalStateException("Not support Json type: ${value.javaClass}")
    //     }
    // }
  }
}
