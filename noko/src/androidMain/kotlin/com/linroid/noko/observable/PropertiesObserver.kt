package com.linroid.noko.observable

import androidx.annotation.Keep
import com.linroid.noko.types.JSObject
import com.linroid.noko.types.JSValue

/**
 * An observer to receive property update events from [JSObject]
 */
@Keep
interface PropertiesObserver {
  fun onPropertyChanged(key: String, value: JSValue)
}
