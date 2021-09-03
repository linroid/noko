package com.linroid.knode.observable

import androidx.annotation.Keep
import com.linroid.knode.js.JSObject
import com.linroid.knode.js.JSValue

/**
 * An observer to receive property update events from [JSObject]
 *
 * @author linroid
 * @since 2/26/21
 */
@Keep
interface PropertiesObserver {
  fun onPropertyChanged(key: String, value: JSValue)
}
