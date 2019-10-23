package com.linroid.knode.js

/**
 * @author linroid
 * @since 2019-10-23
 */
class JSArray(context: JSContext, reference: Long) : JSObject(context, reference), List<JSValue> {
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

    override fun iterator() = object : AbstractIterator<JSValue>() {
        private var count = size
        private var index = 0

        override fun computeNext() {
            if (count == 0) {
                done()
            } else {
                @Suppress("UNCHECKED_CAST")
                setNext(nativeGet(index))
                index += 1
                count--
            }
        }
    }

    override fun lastIndexOf(element: JSValue): Int {
        return nativeLastIndexOf(element)
    }

    override fun listIterator(): ListIterator<JSValue> {
    }

    override fun listIterator(index: Int): ListIterator<JSValue> {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    override fun subList(fromIndex: Int, toIndex: Int): List<JSValue> {
        TODO("not implemented") //To change body of created functions use File | Settings | File Templates.
    }

    private external fun nativeGet(index: Int): JSValue

    private external fun nativeContains(element: JSValue): Boolean

    private external fun nativeContainsAll(elements: Array<JSValue>): Boolean

    private external fun nativeIndexOf(element: JSValue): Int

    private external fun nativeLastIndexOf(element: JSValue): Int

    private external fun nativeSize(): Int
}
