package com.linroid.noko.path

import com.linroid.noko.noko
import java.io.File

/**
 * @author linroid
 * @since 3/21/21
 */
interface FileSystem {

  fun path(file: File): String

  fun file(path: String): File

  fun mount(node: noko)
}
