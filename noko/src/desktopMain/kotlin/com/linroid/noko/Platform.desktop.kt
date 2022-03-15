package com.linroid.noko

import okio.FileSystem
import java.io.File
import java.nio.file.Files
import java.nio.file.Path

actual object Platform {

  actual val fileSystem = FileSystem.SYSTEM
  private val isDebuggerConnected: Boolean = try {
    // System.getProperty("java.vm.info", "").contains("sharing")
    Class.forName("org.junit.Test")
    true
  } catch (e: ClassNotFoundException) {
    false
  }

  actual fun isDebuggerConnected(): Boolean {
    return isDebuggerConnected
  }

  @Suppress("UnsafeDynamicallyLoadedCode")
  actual fun loadNativeLibraries() {
    try {
      System.loadLibrary("noko")
    } catch (error: UnsatisfiedLinkError) {
      val libDir = Files.createTempDirectory("noko")
      val libNoko = extractResourceTo(System.mapLibraryName("noko"), libDir)
      val libNode = extractResourceTo(System.mapLibraryName("node.93"), libDir)
      System.load(libNode.absolutePath)
      System.load(libNoko.absolutePath)
    }
  }

  private fun extractResourceTo(path: String, targetDir: Path): File {
    val stream = Platform::class.java.classLoader.getResourceAsStream(path)
    checkNotNull(stream) { "Couldn't found $path" }
    stream.use { input ->
      val file = File(targetDir.toFile(), File(path).name)
      file.createNewFile()
      file.outputStream().use { output ->
        input.copyTo(output)
      }
      return file
    }
  }
}
