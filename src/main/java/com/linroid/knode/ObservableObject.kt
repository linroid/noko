package com.linroid.knode

import com.linroid.knode.js.JSObject
import com.linroid.knode.observable.PropertyObserver
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow

/**
 * @author linroid
 * @since 2/26/21
 */
abstract class ObservableObject : PropertyObserver {

  // Note: the fields is not thread-safe
  private val fields = HashMap<String, MutableStateFlow<Any?>>()

  override fun onPropertyChanged(key: String, value: Any?) {
    if (BuildConfig.DEBUG) {
      check(fields.containsKey(key)) { "The property($key) is not observing" }
    }
    val stateFlow = fields.getValue(key)
    stateFlow.value = value
  }

  @Suppress("UNCHECKED_CAST")
  protected fun <T> observeProperty(key: String): StateFlow<T?> {
    check(!fields.containsKey(key)) { "The property($key) has been already observed!" }
    val holder = MutableStateFlow<T?>(null)
    fields[key] = holder as MutableStateFlow<Any?>
    return holder
  }

  @Suppress("UNCHECKED_CAST")
  protected fun <T> observeProperty(key: String, defaultValue: T): StateFlow<T> {
    check(!fields.containsKey(key)) { "The property($key) has been already observed!" }
    val holder = MutableStateFlow(defaultValue)
    fields[key] = holder as MutableStateFlow<Any?>
    return holder
  }

  fun bind(obj: JSObject) {
    obj.watch(this, *fields.keys.toTypedArray())
  }
}
