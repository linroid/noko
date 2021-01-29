package com.linroid.knode

import android.util.Log
import com.linroid.knode.js.JSValue
import java.lang.ref.PhantomReference
import java.util.concurrent.CopyOnWriteArraySet

/**
 * Created by linroid on 1/27/21.
 */
class JSValueReference(private val value: JSValue) : PhantomReference<JSValue>(value, ReferenceWatcher.queue) {

  init {
    references.add(this)
  }

  override fun clear() {
    Log.w(TAG, "clear($value)")
    value.close()
    references.remove(this)
    super.clear()
  }

  companion object {
    private const val TAG = "ReferenceWatcher"
    private val references: CopyOnWriteArraySet<JSValueReference> = CopyOnWriteArraySet()
  }
}