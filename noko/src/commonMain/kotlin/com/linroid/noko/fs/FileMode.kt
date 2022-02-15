package com.linroid.noko.fs

enum class FileMode(internal val flags: Int) {
  None(0),
  ReadOnly(1),
  WriteOnly(2),
  ReadWrite(3),
}