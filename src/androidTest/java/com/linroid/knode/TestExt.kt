package com.linroid.knode

import org.junit.Assert
import kotlin.reflect.KClass

/**
 * @author linroid
 * @since 2019/11/27
 */
fun <T : Any> assertInstance(obj: Any, clazz: KClass<T>) {
    Assert.assertTrue("${obj.javaClass} is not an instance of $clazz ", clazz.isInstance(obj))
}