package com.linroid.knode

import android.util.Log
import com.linroid.knode.js.JSObject
import com.linroid.knode.js.JSValue
import java.lang.ref.PhantomReference
import java.util.concurrent.CopyOnWriteArraySet

/**
 * Created by linroid on 1/27/21.
 */
class JSValueReference(value: JSValue, private val cleaner: (Long) -> Unit) : PhantomReference<JSValue>(value, ReferenceWatcher.queue) {
  private val ref = value.reference
  private val identify = value.typeOf() + " - ${value.javaClass} - ${(value as? JSObject)?.keys()?.contentToString()}"

  init {
    references.add(this)
  }

  override fun clear() {
    Log.w(TAG, "clear($ref) $identify")
    cleaner(ref)
    references.remove(this)
    super.clear()
  }

  companion object {
    private const val TAG = "ReferenceWatcher"
    private val references: CopyOnWriteArraySet<JSValueReference> = CopyOnWriteArraySet()
  }
}