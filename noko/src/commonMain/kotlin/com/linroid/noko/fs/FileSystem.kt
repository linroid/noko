package com.linroid.noko.fs

import com.linroid.noko.Noko
import java.io.File

interface FileSystem {

  fun path(file: File): String

  fun file(path: String): File

  fun mount(node: Noko)
}
