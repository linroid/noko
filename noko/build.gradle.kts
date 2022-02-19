import org.jetbrains.kotlin.gradle.dsl.KotlinMultiplatformExtension
import de.undercouch.gradle.tasks.download.Download
import com.android.build.gradle.tasks.ExternalNativeBuildTask

plugins {
  kotlin("multiplatform") version "1.6.10"
  kotlin("plugin.serialization") version "1.6.10"
  id("com.android.library")
  id("de.undercouch.download") version "5.0.1"
  // id("kotlinx-atomicfu")
}

group = "com.linroid.noko"
version = "1.0-SNAPSHOT"

val coroutinesVersion = "1.6.0"

repositories {
  google()
  mavenCentral()
}

// Remove useless source sets
// See https://discuss.kotlinlang.org/t/disabling-androidandroidtestrelease-source-set-in-gradle-kotlin-dsl-script/21448
afterEvaluate {
  extensions.getByType<KotlinMultiplatformExtension>().sourceSets.removeAll { sourceSet ->
    setOf(
      "androidAndroidTestRelease",
      "androidTestFixtures",
      "androidTestFixturesDebug",
      "androidTestFixturesRelease",
    ).contains(sourceSet.name)
  }
}

android {
  compileSdk = 31
  sourceSets["main"].manifest.srcFile("src/androidMain/AndroidManifest.xml")

  defaultConfig {
    minSdk = 24
    targetSdk = 31
    testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    consumerProguardFiles(file("consumer-rules.pro"))
    externalNativeBuild {
      cmake {
        cppFlags += "-std=c++11 -fexceptions"
        arguments += setOf("-DANDROID_ARM_MODE=arm", "-DANDROID_STL=c++_shared")
      }
    }
  }

  compileOptions {
    sourceCompatibility = JavaVersion.VERSION_1_8
    targetCompatibility = JavaVersion.VERSION_1_8
  }

  externalNativeBuild {
    cmake {
      path = file("src/jvmMain/cpp/CMakeLists.txt")
      version = "3.10.2"
    }
  }

  packagingOptions {
    jniLibs {
      keepDebugSymbols += "**/*.so"
    }
  }
}

java {
  sourceCompatibility = JavaVersion.VERSION_1_8
  targetCompatibility = JavaVersion.VERSION_1_8
}


//val desktopJar by project.tasks.registering(Jar::class) {
//  archiveBaseName.set("noko-desktop")
//  from(kotlin.jvm("desktop").compilations["main"].output.allOutputs)
//}

tasks {
  val version = project.property("libnode.version")
  val downloadsDir = File(buildDir, "downloads")
  val osName = System.getProperty("os.name")
  val targetOs = when {
    osName == "Mac OS X" -> "macos"
    osName.startsWith("Win") -> "windows"
    osName.startsWith("Linux") -> "linux"
    else -> error("Unsupported OS: $osName")
  }

  val targetArch = when (val osArch = System.getProperty("os.arch")) {
    "x86_64", "amd64" -> "x86_64"
    "aarch64" -> "arm64"
    else -> error("Unsupported arch: $osArch")
  }

  val prebuiltDir = file("src/jvmMain/cpp/prebuilt")
  val targetHostPrebuiltDir = File(prebuiltDir, "$targetOs/$targetArch")
  val cmakeDir = File(buildDir, "$targetOs/cmake")
  cmakeDir.mkdirs()

  val downloadAndroidPrebuilt by registering(Download::class) {
    src("https://github.com/linroid/libnode/releases/download/$version/libnode-v16.14.0-android.zip")
    dest(File(downloadsDir, "libnode.android.zip"))
  }

  val targetAndroidPrebuiltDir = file("src/jvmMain/cpp/prebuilt/android")
  val prepareAndroidPrebuilt by registering(Copy::class) {
    dependsOn(downloadAndroidPrebuilt)
    from(zipTree(File(downloadsDir, "libnode.android.zip")))
    into(targetAndroidPrebuiltDir)
  }

  val downloadHostPrebuilt by registering(Download::class) {
    src("https://github.com/linroid/libnode/releases/download/$version/libnode-$version-$targetOs-$targetArch.zip")
    dest(File(downloadsDir, "libnode.$targetOs.zip"))
  }

  val prepareHostPrebuilt by registering(Copy::class) {
    dependsOn(downloadHostPrebuilt)
    from(zipTree(File(downloadsDir, "libnode.$targetOs.zip")))
    into(targetHostPrebuiltDir)
  }

  val hostCmake by registering(Exec::class) {
    workingDir(cmakeDir)
    commandLine("cmake", file("src/jvmMain/cpp/"))
  }

  val hostCmakeBuild by registering(Exec::class) {
    if (!targetHostPrebuiltDir.exists()) {
      dependsOn(prepareHostPrebuilt)
    }
    dependsOn(hostCmake)
    workingDir(cmakeDir)
    commandLine("make")
    doLast {
      val libNoko = cmakeDir.listFiles().find { it.name.startsWith("libnoko") }
      checkNotNull(libNoko) { "Couldn't find file: libnoko" }
      val libNode = File(targetHostPrebuiltDir, "lib").listFiles().find {
        it.name.startsWith("libnode")
      }
      checkNotNull(libNode) { "Couldn't find file: libnode" }
      project.tasks.withType(Jar::class).forEach {
        it.from(libNoko)
        it.from(libNode)
      }
    }
  }

  named("clean").configure {
    doLast {
      delete(prebuiltDir)
    }
  }

  afterEvaluate {
    getByName("compileKotlinDesktop").dependsOn(hostCmakeBuild)
    if (!targetAndroidPrebuiltDir.exists()) {
      tasks.withType(ExternalNativeBuildTask::class).forEach {
        it.dependsOn(prepareAndroidPrebuilt)
      }
    }
  }
}

kotlin {
  android {}

  jvm {
    compilations.all {
      kotlinOptions.jvmTarget = "1.8"
    }
    testRuns["test"].executionTask.configure {
      useJUnit()
    }
  }

  jvm("desktop") {
    compilations.all {
      kotlinOptions.jvmTarget = "11"
    }
    testRuns["test"].executionTask.configure {
      useJUnit()
    }
  }

  sourceSets {
    val commonMain by getting {
      dependencies {
        implementation(kotlin("stdlib"))
        implementation("com.squareup.okio:okio:3.0.0")
        implementation("org.jetbrains.kotlinx:kotlinx-serialization-json:1.3.2")
        implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core:$coroutinesVersion")
        implementation("org.jetbrains.kotlinx:atomicfu:0.17.1")
      }
    }
    val commonTest by getting {
      dependencies {
        implementation(kotlin("test"))
      }
    }

    val jvmMain by getting
    val jvmTest by getting

    val desktopMain by getting {
      dependsOn(jvmMain)
      dependencies {
        implementation(kotlin("stdlib"))
        implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core-jvm:$coroutinesVersion")
      }
    }
    val desktopTest by getting {
    }

    val androidMain by getting {
      dependsOn(jvmMain)
      dependencies {
        implementation("org.jetbrains.kotlinx:kotlinx-coroutines-android:$coroutinesVersion")
      }
    }
    val androidTest by getting {
      dependsOn(jvmTest)
      dependencies {
        implementation("junit:junit:4.13.2")
      }
    }
  }
}
