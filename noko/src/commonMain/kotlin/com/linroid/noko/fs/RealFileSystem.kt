package com.linroid.noko.fs

class RealFileSystem : FileSystem() {

  override fun path(file: String): String {
    return file
  }

  override fun file(path: String): String {
    return path
  }
}
