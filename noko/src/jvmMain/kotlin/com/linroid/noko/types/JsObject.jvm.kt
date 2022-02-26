package com.linroid.noko.types

import com.linroid.noko.Node
import com.linroid.noko.annotation.ForNative
import com.linroid.noko.observable.PropertiesObserver
import kotlin.reflect.KClass

open class JsObject : JsValue {

  @ForNative
  protected constructor(node: Node, ptr: Long) : super(node, ptr)

  constructor(node: Node) : super(node, 0) {
    node.checkThread()
    nativeNew()
    addBinds()
  }

  private fun addBinds() {
    if (this::class == JsObject::class) {
      return
    }
    // val className = this::class.simpleName
    // this::class.methods
    //   .filter { it.isAnnotationPresent(JSName::class.java) }
    //   .forEach { method ->
    //     method.isAccessible = true
    //     val bind = method.getAnnotation(JSName::class.java)!!
    //     val name = bind.name.ifEmpty { method.name }
    //     set(name, object : JsFunction(noko, name) {
    //       override fun onCall(receiver: JsValue, parameters: Array<out JsValue>): JsValue {
    //         val result = try {
    //           method.invoke(this@JsObject, *convertParameters(parameters, method.parameterTypes))
    //         } catch (error: InvocationTargetException) {
    //           noko.throwError("Unexpected error occurred during call '${className}#$name()': ${error.targetException.message}")
    //         } catch (error: Exception) {
    //           noko.throwError("Unexpected error occurred during call '${className}#$name()': ${error.message}")
    //         }
    //         return from(noko, result)
    //       }
    //     })
    //   }
  }

  private fun convertParameters(
    parameters: Array<out JsValue>,
    parameterTypes: Array<KClass<*>>
  ): Array<Any?> {
    val argc = parameterTypes.size
    return Array(argc) { i ->
      val value = parameters[i]
      val type = parameterTypes[i]
      value.toType(type)
    }
  }

  fun has(key: String): Boolean {
    return nativeHas(key)
  }

  fun set(key: String, value: Any?) {
    nativeSet(key, from(node, value))
  }

  inline fun <reified T : Any> get(key: String): T {
    return get(key, T::class)
      ?: throw IllegalStateException("get $key from $this shouldn't return null")
  }

  inline fun <reified T : Any> opt(key: String): T? {
    return get(key, T::class)
  }

  fun <T : Any> get(key: String, clazz: KClass<T>): T? {
    node.checkThread()
    val value = nativeGet(key)
    return value.toType(clazz)
  }

  fun delete(key: String) {
    nativeDelete(key)
  }

  fun keys(): Array<String> {
    return nativeKeys()
  }

  /**
   * Watch some properties to get notified once they are changed
   */
  fun watch(observer: PropertiesObserver, vararg properties: String) {
    nativeWatch(properties, observer)
  }

  private external fun nativeGet(key: String): JsValue
  private external fun nativeKeys(): Array<String>
  private external fun nativeHas(key: String): Boolean
  private external fun nativeSet(key: String, value: JsValue?)
  private external fun nativeDelete(key: String)
  private external fun nativeNew()
  private external fun nativeWatch(properties: Array<out String>, observer: PropertiesObserver)
}
