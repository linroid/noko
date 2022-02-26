package com.linroid.noko.annotation

/**
 * Specific the exported name for js runtime
 */
@Retention(AnnotationRetention.SOURCE)
@Target(AnnotationTarget.FIELD, AnnotationTarget.FUNCTION)
annotation class JsName(val name: String)