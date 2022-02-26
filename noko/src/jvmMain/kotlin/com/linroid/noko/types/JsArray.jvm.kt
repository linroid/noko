package com.linroid.noko.types

import com.linroid.noko.Node
import com.linroid.noko.annotation.ForNative

class JsArray : JsObject, MutableList<JsValue> {

  @ForNative
  private constructor(node: Node, ptr: Long) : super(node, ptr)

  constructor(node: Node, data: Iterator<*>) : super(node, 0) {
    nativeNew()
    val array = ArrayList<JsValue>()
    data.forEach {
      array.add(from(node, it))
    }
    addAll(array)
  }

  override val size: Int
    get() = nativeSize()

  override fun contains(element: JsValue): Boolean {
    return nativeContains(element)
  }

  override fun containsAll(elements: Collection<JsValue>): Boolean {
    return nativeContainsAll(elements.toTypedArray())
  }

  override fun get(index: Int): JsValue {
    return nativeGet(index)
  }

  override fun indexOf(element: JsValue): Int {
    return nativeIndexOf(element)
  }

  override fun isEmpty(): Boolean {
    return nativeSize() == 0
  }

  override fun iterator(): MutableIterator<JsValue> {
    return object : MutableIterator<JsValue> {
      /** the index of the item that will be returned on the next call to [next]`()` */
      private var index = 0

      override fun hasNext(): Boolean = index < size

      override fun next(): JsValue {
        if (!hasNext()) throw NoSuchElementException()
        return get(index++)
      }

      override fun remove() {
      }
    }

  }

  override fun listIterator(): MutableListIterator<JsValue> {
    TODO("not implemented")
  }

  override fun listIterator(index: Int): MutableListIterator<JsValue> {
    TODO("not implemented")
  }

  override fun subList(fromIndex: Int, toIndex: Int): MutableList<JsValue> {
    TODO("not implemented")
  }

  override fun add(element: JsValue): Boolean {
    return nativeAdd(element)
  }

  override fun add(index: Int, element: JsValue) {
    nativeAddAt(index, element)
  }

  override fun addAll(index: Int, elements: Collection<JsValue>): Boolean {
    return nativeAddAllAt(index, elements.toTypedArray())
  }

  override fun addAll(elements: Collection<JsValue>): Boolean {
    return nativeAddAll(elements.toTypedArray())
  }

  override fun clear() {
    nativeClear()
  }

  override fun remove(element: JsValue): Boolean {
    return nativeRemove(element)
  }

  override fun removeAll(elements: Collection<JsValue>): Boolean {
    elements.forEach {
      if (contains(it)) {
        if (!remove(it)) {
          return false
        }
      }
    }
    return true
  }

  override fun removeAt(index: Int): JsValue {
    return nativeRemvoeAt(index)
  }

  override fun retainAll(elements: Collection<JsValue>): Boolean {
    TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
  }

  override fun set(index: Int, element: JsValue): JsValue {
    return nativeSetAt(index, element)
  }

  override fun lastIndexOf(element: JsValue): Int {
    return nativeLastIndexOf(element)
  }

  private external fun nativeRemvoeAt(index: Int): JsValue
  private external fun nativeRemove(element: JsValue): Boolean
  private external fun nativeGet(index: Int): JsValue
  private external fun nativeSetAt(index: Int, element: JsValue): JsValue
  private external fun nativeAdd(value: JsValue): Boolean
  private external fun nativeAddAt(index: Int, value: JsValue): JsValue
  private external fun nativeContains(element: JsValue): Boolean
  private external fun nativeContainsAll(elements: Array<JsValue>): Boolean
  private external fun nativeIndexOf(element: JsValue): Int
  private external fun nativeLastIndexOf(element: JsValue): Int
  private external fun nativeSize(): Int
  private external fun nativeAddAll(elements: Array<JsValue>): Boolean
  private external fun nativeAddAllAt(index: Int, elements: Array<JsValue>): Boolean
  private external fun nativeClear()
  private external fun nativeNew()
}
