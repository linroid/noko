package com.linroid.knode.observable

import androidx.annotation.Keep
import com.linroid.knode.js.JSObject
import com.linroid.knode.js.JSValue

/**
 * An observer will be called when the property of [JSObject] is changed
 *
 * @author linroid
 * @since 2/26/21
 */
@Keep
interface PropertyObserver {
  fun onPropertyChanged(key: String, value: JSValue)
}
