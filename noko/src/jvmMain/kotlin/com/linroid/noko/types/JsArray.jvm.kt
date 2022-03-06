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
    return indexOf(element) >= 0
  }

  override fun containsAll(elements: Collection<Any?>): Boolean {
    return elements.all { contains(it) }
  }

  override fun get(index: Int): Any? {
    return nativeGet(index)
  }

  override fun indexOf(element: Any?): Int {
    return (0 until size).find { get(it) == element } ?: -1
  }

  override fun isEmpty(): Boolean {
    return nativeSize() == 0
  }

  override fun iterator(): MutableIterator<Any?> {
    return object : MutableIterator<Any?> {
      /** The index of the item that will be returned on the next call to [next]`()` */
      private var index = 0

      override fun hasNext(): Boolean = index < size

      override fun next(): Any? {
        if (!hasNext()) throw NoSuchElementException()
        return get(index++)
      }

      override fun remove() {
        index--
        removeAt(index)
      }
    }
  }

  override fun listIterator(): MutableListIterator<Any?> {
    TODO("Not implemented")
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
    elements.forEachIndexed { offset, element ->
      nativeAddAt(index + offset, element)
    }
    return elements.isNotEmpty()
  }

  override fun addAll(elements: Collection<Any?>): Boolean {
    return nativeAddAll(elements.toTypedArray())
  }

  override fun clear() {
    nativeClear()
  }

  override fun remove(element: Any?): Boolean {
    return nativeRemoveAt(indexOf(element)) == element
  }

  override fun removeAll(elements: Collection<Any?>): Boolean {
    return elements.all { remove(it) }
  }

  override fun removeAt(index: Int): Any? {
    return nativeRemoveAt(index)
  }

  override fun retainAll(elements: Collection<Any?>): Boolean {
    TODO("not implemented")
  }

  override fun set(index: Int, element: Any?): Any? {
    return nativeSetAt(index, element)
  }

  override fun lastIndexOf(element: Any?): Int {
    return (size - 1 downTo 0).find { get(it) == element } ?: -1
  }

  private external fun nativeRemoveAt(index: Int): Any?
  private external fun nativeGet(index: Int): Any?
  private external fun nativeSetAt(index: Int, element: Any?): Any?
  private external fun nativeAdd(value: Any?): Boolean
  private external fun nativeAddAt(index: Int, value: Any?)
  private external fun nativeSize(): Int
  private external fun nativeAddAll(elements: Array<Any?>): Boolean
  private external fun nativeClear()

  companion object {
    @JvmStatic
    private external fun nativeNew(): NativePointer
  }
}
