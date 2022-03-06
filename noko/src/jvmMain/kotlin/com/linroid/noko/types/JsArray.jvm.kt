package com.linroid.noko.types

import com.linroid.noko.NativePointer
import com.linroid.noko.Node
import com.linroid.noko.NullNativePointer

actual class JsArray : JsObject, MutableList<Any?> {

  actual constructor(node: Node, data: Iterable<*>) : super(node, nativeNew()) {
    if (data.any()) {
      addAll(data)
    }
  }

  internal actual constructor(node: Node, pointer: NativePointer) : super(node, pointer)

  override val size: Int
    get() = nativeSize()

  override fun contains(element: Any?): Boolean {
    return nativeContains(element)
  }

  override fun containsAll(elements: Collection<Any?>): Boolean {
    return nativeContainsAll(elements.toTypedArray())
  }

  override fun get(index: Int): Any? {
    return nativeGet(index)
  }

  override fun indexOf(element: Any?): Int {
    return nativeIndexOf(element)
  }

  override fun isEmpty(): Boolean {
    return nativeSize() == 0
  }

  override fun iterator(): MutableIterator<Any?> {
    return object : MutableIterator<Any?> {
      /** the index of the item that will be returned on the next call to [next]`()` */
      private var index = 0

      override fun hasNext(): Boolean = index < size

      override fun next(): Any? {
        if (!hasNext()) throw NoSuchElementException()
        return get(index++)
      }

      override fun remove() {
      }
    }
  }

  override fun listIterator(): MutableListIterator<Any?> {
    TODO("not implemented")
  }

  override fun listIterator(index: Int): MutableListIterator<Any?> {
    TODO("not implemented")
  }

  override fun subList(fromIndex: Int, toIndex: Int): MutableList<Any?> {
    TODO("not implemented")
  }

  override fun add(element: Any?): Boolean {
    return nativeAdd(element)
  }

  override fun add(index: Int, element: Any?) {
    nativeAddAt(index, element)
  }

  override fun addAll(index: Int, elements: Collection<Any?>): Boolean {
    return nativeAddAllAt(index, elements.toTypedArray())
  }

  override fun addAll(elements: Collection<Any?>): Boolean {
    return nativeAddAll(elements.toTypedArray())
  }

  override fun clear() {
    nativeClear()
  }

  override fun remove(element: Any?): Boolean {
    return nativeRemove(element)
  }

  override fun removeAll(elements: Collection<Any?>): Boolean {
    elements.forEach {
      if (contains(it)) {
        if (!remove(it)) {
          return false
        }
      }
    }
    return true
  }

  override fun removeAt(index: Int): Any? {
    return nativeRemoveAt(index)
  }

  override fun retainAll(elements: Collection<Any?>): Boolean {
    TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
  }

  override fun set(index: Int, element: Any?): Any? {
    return nativeSetAt(index, element)
  }

  override fun lastIndexOf(element: Any?): Int {
    return nativeLastIndexOf(element)
  }

  private external fun nativeRemoveAt(index: Int): Any?
  private external fun nativeRemove(element: Any?): Boolean
  private external fun nativeGet(index: Int): Any?
  private external fun nativeSetAt(index: Int, element: Any?): Any?
  private external fun nativeAdd(value: Any?): Boolean
  private external fun nativeAddAt(index: Int, value: Any?): Any?
  private external fun nativeContains(element: Any?): Boolean
  private external fun nativeContainsAll(elements: Array<Any?>): Boolean
  private external fun nativeIndexOf(element: Any?): Int
  private external fun nativeLastIndexOf(element: Any?): Int
  private external fun nativeSize(): Int
  private external fun nativeAddAll(elements: Array<Any?>): Boolean
  private external fun nativeAddAllAt(index: Int, elements: Array<Any?>): Boolean
  private external fun nativeClear()

  companion object {
    @JvmStatic
    private external fun nativeNew(): NativePointer
  }
}
