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
        data.forEach {
            add(from(context, it))
        }
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
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    override fun listIterator(): MutableListIterator<JSValue> {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    override fun listIterator(index: Int): MutableListIterator<JSValue> {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    override fun subList(fromIndex: Int, toIndex: Int): MutableList<JSValue> {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    override fun add(element: JSValue): Boolean {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    override fun add(index: Int, element: JSValue) {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    override fun addAll(index: Int, elements: Collection<JSValue>): Boolean {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    override fun addAll(elements: Collection<JSValue>): Boolean {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    override fun clear() {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    override fun remove(element: JSValue): Boolean {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    override fun removeAll(elements: Collection<JSValue>): Boolean {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    override fun removeAt(index: Int): JSValue {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    override fun retainAll(elements: Collection<JSValue>): Boolean {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    override fun set(index: Int, element: JSValue): JSValue {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    override fun lastIndexOf(element: JSValue): Int {
        return nativeLastIndexOf(element)
    }

    private external fun nativeGet(index: Int): JSValue
    private external fun nativeContains(element: JSValue): Boolean
    private external fun nativeContainsAll(elements: Array<JSValue>): Boolean
    private external fun nativeIndexOf(element: JSValue): Int
    private external fun nativeLastIndexOf(element: JSValue): Int
    private external fun nativeSize(): Int
    private external fun nativeNew()
}
