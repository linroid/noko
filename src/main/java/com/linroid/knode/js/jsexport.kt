package com.linroid.knode.js

import kotlin.reflect.KClass

/**
 * @author linroid
 * @since 2019/11/1
 */
annotation class jsexport(val type: KClass<*> = Any::class)