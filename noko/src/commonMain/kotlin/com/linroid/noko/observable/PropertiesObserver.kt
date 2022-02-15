package com.linroid.noko.observable

import com.linroid.noko.types.JSObject
import com.linroid.noko.types.JSValue

/**
 * An observer to receive property update events from [JSObject]
 */
interface PropertiesObserver {
  fun onPropertyChanged(key: String, value: JSValue)
}
