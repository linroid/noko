package com.linroid.noko.observable

import androidx.annotation.Keep
import com.linroid.noko.js.JSObject
import com.linroid.noko.js.JSValue

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
