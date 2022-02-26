package com.linroid.noko.observable

import com.linroid.noko.types.JsObject
import com.linroid.noko.types.JsValue

/**
 * An observer to receive property update events from [JsObject]
 */
interface PropertiesObserver {
  fun onPropertyChanged(key: String, value: JsValue)
}
