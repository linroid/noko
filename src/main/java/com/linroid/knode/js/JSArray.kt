package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-23
 */
class JSArray : JSObject, MutableList<JSValue> {

  @NativeConstructor
  private constructor(context: JSContext, reference: Long) : super(context, reference)

  constructor(context: JSContext, data: Iterator<*>) : super(context, 0) {
    nativeNew()
    val array = ArrayList<JSValue>()
    data.forEach {
      array.add(from(context, it))
    }
    addAll(array)
  }

  override val size: Int
    get() = nativeSize()

  override fun contains(element: JSValue): Boolean {
    return nativeContains(element)
  }

  override fun containsAll(elements: Collection<JSValue>): Boolean {
    return nativeContainsAll(elements.toTypedArray())
  }

  override fun get(index: Int): JSValue {
    return nativeGet(index)
  }

  override fun indexOf(element: JSValue): Int {
    return nativeIndexOf(element)
  }

  override fun isEmpty(): Boolean {
    return nativeSize() == 0
  }

  override fun iterator(): MutableIterator<JSValue> {
    return object : MutableIterator<JSValue> {
      /** the index of the item that will be returned on the next call to [next]`()` */
      private var index = 0

      override fun hasNext(): Boolean = index < size

      override fun next(): JSValue {
        if (!hasNext()) throw NoSuchElementException()
        return get(index++)
      }

      override fun remove() {
      }
    }

  }

  override fun listIterator(): MutableListIterator<JSValue> {
    TODO("not implemented")
  }

  override fun listIterator(index: Int): MutableListIterator<JSValue> {
    TODO("not implemented")
  }

  override fun subList(fromIndex: Int, toIndex: Int): MutableList<JSValue> {
    TODO("not implemented")
  }

  override fun add(element: JSValue): Boolean {
    return nativeAdd(element)
  }

  override fun add(index: Int, element: JSValue) {
    nativeAddAt(index, element)
  }

  override fun addAll(index: Int, elements: Collection<JSValue>): Boolean {
    return nativeAddAllAt(index, elements.toTypedArray())
  }

  override fun addAll(elements: Collection<JSValue>): Boolean {
    return nativeAddAll(elements.toTypedArray())
  }

  override fun clear() {
    nativeClear()
  }

  override fun remove(element: JSValue): Boolean {
    return nativeRemove(element)
  }

  override fun removeAll(elements: Collection<JSValue>): Boolean {
    elements.forEach {
      if (contains(it)) {
        if (!remove(it)) {
          return false
        }
      }
    }
    return true
  }

  override fun removeAt(index: Int): JSValue {
    return nativeRemvoeAt(index)
  }

  override fun retainAll(elements: Collection<JSValue>): Boolean {
    TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
  }

  override fun set(index: Int, element: JSValue): JSValue {
    return nativeSetAt(index, element)
  }

  override fun lastIndexOf(element: JSValue): Int {
    return nativeLastIndexOf(element)
  }

  private external fun nativeRemvoeAt(index: Int): JSValue
  private external fun nativeRemove(element: JSValue): Boolean
  private external fun nativeGet(index: Int): JSValue
  private external fun nativeSetAt(index: Int, element: JSValue): JSValue
  private external fun nativeAdd(value: JSValue): Boolean
  private external fun nativeAddAt(index: Int, value: JSValue): JSValue
  private external fun nativeContains(element: JSValue): Boolean
  private external fun nativeContainsAll(elements: Array<JSValue>): Boolean
  private external fun nativeIndexOf(element: JSValue): Int
  private external fun nativeLastIndexOf(element: JSValue): Int
  private external fun nativeSize(): Int
  private external fun nativeAddAll(elements: Array<JSValue>): Boolean
  private external fun nativeAddAllAt(index: Int, elements: Array<JSValue>): Boolean
  private external fun nativeClear()
  private external fun nativeNew()
}
