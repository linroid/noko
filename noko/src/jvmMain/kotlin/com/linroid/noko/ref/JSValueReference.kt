package com.linroid.noko.ref

import com.linroid.noko.types.JsValue
import java.lang.ref.PhantomReference
import java.util.concurrent.CopyOnWriteArraySet

class JSValueReference(
  value: JsValue,
  private val cleaner: (Long) -> Unit
) : PhantomReference<JsValue>(value, ReferenceWatcher.queue) {
  
  private val ref = value.pointer

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
