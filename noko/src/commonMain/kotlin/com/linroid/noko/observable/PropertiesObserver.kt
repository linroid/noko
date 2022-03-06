package com.linroid.noko.observable

import com.linroid.noko.types.JsObject

/**
 * An observer to receive property update events from [JsObject]
 */
interface PropertiesObserver {
  fun onPropertyChanged(key: String, value: Any?)
}
