package com.linroid.noko

import org.junit.Assert
import kotlin.reflect.KClass

fun <T : Any> assertInstance(obj: Any, clazz: KClass<T>) {
  Assert.assertTrue("${obj.javaClass} is not an instance of $clazz ", clazz.isInstance(obj))
}