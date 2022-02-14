package com.linroid.knode

import android.system.Os
import androidx.test.ext.junit.runners.AndroidJUnit4
import androidx.test.platform.app.InstrumentationRegistry
import org.junit.Assert.assertEquals
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import java.io.File
import java.util.concurrent.TimeUnit
import kotlin.concurrent.thread

/**
 * @author linroid
 * @since 2020/4/18
 */
@RunWith(AndroidJUnit4::class)
class ExecutableTest {
  private lateinit var file: File

  @Before
  fun setup() {
    val ctx = InstrumentationRegistry.getInstrumentation().targetContext
    val binDir = File(ctx.cacheDir, "bin")
    if (!binDir.exists()) {
      binDir.mkdirs()
    }
    // println(ctx.applicationInfo.nativeLibraryDir)
    // file = KNode.extractExecutable(ctx, binDir)
    // assert(file.exists())
    //
    val cmd = File(binDir, "node")

    KNode.mountExecutable(ctx, cmd)
    assert(Os.getenv("PATH").split(':').contains(binDir.absolutePath))
    assert(Os.getenv("LD_LIBRARY_PATH").split(':').contains(ctx.applicationInfo.nativeLibraryDir))
    file = File(ctx.applicationInfo.nativeLibraryDir, "node.so")
  }

  @Test
  fun execute() {
    val builder = ProcessBuilder("sh")
    builder.redirectErrorStream(true)
    builder.directory(file.parentFile)
    val process = builder.start()
    val writer = process.outputStream.writer()
    thread {
      val reader = process.inputStream.reader()
      reader.forEachLine {
        println(it)
      }
    }
    writer.write("env\n")
    writer.write("alias node=$file\n")
    writer.write("chmod 777 $file\n")
    writer.write("ls -la ${file.parent}\n")
    writer.write("node -v\n")
    writer.write("exit\n")
    writer.flush()
    process.waitFor(3, TimeUnit.SECONDS)
    assertEquals(0, process.exitValue())
  }
}
