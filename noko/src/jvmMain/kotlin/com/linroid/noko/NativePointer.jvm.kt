package com.linroid.noko

actual typealias NativePointer = Long

actual val NullNativePointer: NativePointer
  get() = 0L
