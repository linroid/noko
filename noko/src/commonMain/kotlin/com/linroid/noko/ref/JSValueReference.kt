package com.linroid.noko.ref

import com.linroid.noko.types.JSValue
import java.lang.ref.PhantomReference
import java.util.concurrent.CopyOnWriteArraySet

class JSValueReference(value: JSValue, private val cleaner: (Long) -> Unit) : PhantomReference<JSValue>(value,
  ReferenceWatcher.queue
) {
  private val ref = value.nPtr

  init {
    references.add(this)
  }

  override fun clear() {
    cleaner(ref)
    references.remove(this)
    super.clear()
  }

  companion object {
    private const val TAG = "ReferenceWatcher"
    private val references: CopyOnWriteArraySet<JSValueReference> = CopyOnWriteArraySet()
  }
}