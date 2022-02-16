package com.linroid.noko.types

import com.linroid.noko.Noko
import com.linroid.noko.annotation.JSName
import com.linroid.noko.annotation.ForNative
import com.linroid.noko.observable.PropertiesObserver
import java.lang.reflect.InvocationTargetException

open class JSObject : JSValue {

  @ForNative
  protected constructor(noko: Noko, nPtr: Long) : super(noko, nPtr)

  constructor(noko: Noko) : super(noko, 0) {
    noko.checkThread()
    nativeNew()
    addBinds()
  }

  private fun addBinds() {
    if (javaClass == JSObject::class.java) {
      return
    }
    val className = javaClass.simpleName
    javaClass.methods
      .filter { it.isAnnotationPresent(JSName::class.java) }
      .forEach { method ->
        method.isAccessible = true
        val bind = method.getAnnotation(JSName::class.java)!!
        val name = bind.name.ifEmpty { method.name }
        set(name, object : JSFunction(noko, name) {
          override fun onCall(receiver: JSValue, parameters: Array<out JSValue>): JSValue {
            val result = try {
              method.invoke(this@JSObject, *convertParameters(parameters, method.parameterTypes))
            } catch (error: InvocationTargetException) {
              noko.throwError("Unexpected error occurred during call '${className}#$name()': ${error.targetException.message}")
            } catch (error: Exception) {
              noko.throwError("Unexpected error occurred during call '${className}#$name()': ${error.message}")
            }
            return from(noko, result)
          }
        })
      }
  }

  private fun convertParameters(
    parameters: Array<out JSValue>,
    parameterTypes: Array<Class<*>>
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
    nativeSet(key, from(noko, value))
  }

  inline fun <reified T> get(key: String): T {
    return get(key, T::class.java)
      ?: throw IllegalStateException("get $key from $this shouldn't return null")
  }

  inline fun <reified T> opt(key: String): T? {
    return get(key, T::class.java)
  }

  fun <T> get(key: String, clazz: Class<T>): T? {
    noko.checkThread()
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

  private external fun nativeGet(key: String): JSValue
  private external fun nativeKeys(): Array<String>
  private external fun nativeHas(key: String): Boolean
  private external fun nativeSet(key: String, value: JSValue?)
  private external fun nativeDelete(key: String)
  private external fun nativeNew()
  private external fun nativeWatch(properties: Array<out String>, observer: PropertiesObserver)
}