import com.android.build.gradle.LibraryExtension
import de.undercouch.gradle.tasks.download.Download
import org.jetbrains.kotlin.gradle.dsl.KotlinMultiplatformExtension
import com.android.build.gradle.tasks.ExternalNativeBuildJsonTask

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

val nodeVersion = project.property("libnode.version")
val downloadsDir = File(buildDir, "downloads")
val prebuiltRoot = file("src/jvmMain/cpp/prebuilt")

enum class Os {
  Android, Linux, Windows, MacOS;

  fun lowercase(): String {
    return toString().toLowerCase()
  }
}

enum class Arch {
  X86, X64, Arm, Arm64;

  fun lowercase(): String {
    return toString().toLowerCase()
  }
}

val osName = System.getProperty("os.name")!!
val hostOs = when {
  osName == "Mac OS X" -> Os.MacOS
  osName.startsWith("Win") -> Os.Windows
  osName.startsWith("Linux") -> Os.Linux
  else -> error("Unsupported OS: $osName")
}

val hostArch = when (val osArch = System.getProperty("os.arch")) {
  "x86_64", "amd64" -> Arch.X64
  "aarch64" -> Arch.Arm64
  else -> error("Unsupported arch: $osArch")
}

val hostCmakeDir = File(buildDir, "cmake/${hostOs.lowercase()}/${hostArch.lowercase()}")
val hostPrebuiltDir = File(prebuiltRoot, "${hostOs.lowercase()}/${hostArch.lowercase()}")
val hostPrebuiltLibDir = File(hostPrebuiltDir, "lib")

tasks {
  named("clean").configure {
    doLast {
      delete(prebuiltRoot)
    }
  }
  afterEvaluate {
    val prebuiltTasks = Arch.values().map { arch ->
      createPrebuiltTask(Os.Android, arch)
    }
    withType<ExternalNativeBuildJsonTask> {
      prebuiltTasks.forEach { dependsOn(it) }
      this.property("abi")
    }
  }

  withType<Test> {
    testLogging {
      outputs.upToDateWhen { false }
      showStandardStreams = true
    }
  }
}

fun createPrebuiltTask(os: Os, arch: Arch): TaskProvider<Copy> {
  val filename = "libnode-$nodeVersion-${os.lowercase()}-${arch.lowercase()}.zip"
  val localFile = File(downloadsDir, filename)
  val prebuiltDir = File(prebuiltRoot, "${os.lowercase()}/${arch.lowercase()}")

  val downloadTask = tasks.register("prebuiltDownload${os}${arch}", Download::class) {
    src("https://github.com/linroid/libnode/releases/download/$nodeVersion/$filename")
    onlyIf { !dest.exists() }
    onlyIfModified(true)
    dest(localFile)
  }
  val unzipTask = tasks.register("prebuiltUnzip${os}${arch}", Copy::class) {
    dependsOn(downloadTask)
    from(zipTree(localFile))
    onlyIf { !destinationDir.exists() || destinationDir.listFiles().isEmpty() }
    into(prebuiltDir)
  }
  return unzipTask
}

val hostCmakeTask by lazy {
  val cmakeConfigureTask = tasks.register("configureCMake${hostOs}${hostArch}", Exec::class) {
    workingDir(hostCmakeDir)
    commandLine(
      "cmake", file("src/jvmMain/cpp"),
      "-DTARGET_OS=${hostOs.lowercase()}",
      "-DTARGET_ARCH=${hostArch.lowercase()}",
    )
  }.get()

  cmakeConfigureTask.dependsOn(createPrebuiltTask(hostOs, hostArch))
  tasks.register("buildCMake${hostOs}${hostArch}", Exec::class) {
    hostCmakeDir.mkdirs()
    dependsOn(cmakeConfigureTask)
    workingDir(hostCmakeDir)
    commandLine("make")
    doLast {
      val libNoko = hostCmakeDir.listFiles()!!.find { it.name.startsWith("libnoko") }
      checkNotNull(libNoko) { "Couldn't find file: libnoko" }
      val libNode = hostPrebuiltLibDir.listFiles()!!.find {
        it.name.startsWith("libnode")
      }
      checkNotNull(libNode) { "Couldn't find file: libnode" }
      tasks.withType(Jar::class).forEach {
        it.from(libNode)
        it.from(libNoko)
      }
    }
  }
}

kotlin {
  android {}

  jvm("desktop") {
    compilations.all {
      kotlinOptions.jvmTarget = "11"
      compileKotlinTask.dependsOn(hostCmakeTask)
    }
    testRuns["test"].executionTask.configure {
      useJUnit()
      jvmArgs("-Djava.library.path=$hostCmakeDir:$hostPrebuiltLibDir")
      reports {
        junitXml.required.set(true)
        html.required.set(true)
      }
    }
  }

  sourceSets {
    val mockkVersion = "1.12.3"
    val commonMain by getting {
      dependencies {
        implementation(kotlin("stdlib"))
        implementation("com.squareup.okio:okio:3.0.0")
        implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core:$coroutinesVersion")
        implementation("org.jetbrains.kotlinx:atomicfu:0.17.1")
      }
    }
    val commonTest by getting {
      dependencies {
        implementation("io.mockk:mockk-common:${mockkVersion}")
        implementation(kotlin("test"))
        implementation("org.jetbrains.kotlinx:kotlinx-serialization-json:1.3.2")
      }
    }

    val jvmMain by creating {
      dependsOn(commonMain)
      dependencies {
        implementation("org.jetbrains.kotlinx:atomicfu-jvm:0.17.1")
        implementation("com.squareup.okio:okio-jvm:3.0.0")
        implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core-jvm:$coroutinesVersion")
      }
    }

    val jvmTest by creating {
      dependsOn(commonTest)
      dependencies {
        implementation(kotlin("test"))
      }
    }

    val desktopMain by getting {
      dependsOn(jvmMain)
    }

    val desktopTest by getting {
      dependsOn(jvmTest)
      dependencies {
        implementation("io.mockk:mockk:${mockkVersion}")
        implementation("io.mockk:mockk-agent-jvm:${mockkVersion}")
      }
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
    val androidAndroidTest by getting {
      dependsOn(jvmTest)
      dependencies {
        implementation("androidx.test:runner:1.4.0")
        implementation("io.mockk:mockk-android:${mockkVersion}")
      }
    }
  }
}

fun configureSourceSets() {
  val libraryExtension = project.extensions.findByType<LibraryExtension>()!!
  // TODO: b/148416113: AGP doesn't know about Kotlin-MPP's sourceSets yet, so add
  // them to its source directories (this fixes lint, and code completion in
  // Android Studio on versions >= 4.0canary8)
  libraryExtension.apply {
    sourceSets.findByName("main")?.apply {
      kotlin.srcDirs("src/androidMain/kotlin")
      res.srcDirs("src/androidMain/res")
      assets.srcDirs("src/androidMain/assets")
      // Keep Kotlin files in java source sets so the source set is not empty when
      // running unit tests which would prevent the tests from running in CI.
      java.includes.add("**/*.kt")
    }
    sourceSets.findByName("test")?.apply {
      kotlin.srcDirs("src/androidTest/kotlin")
      res.srcDirs("src/commonTest/res", "src/jvmTest/res")
      // Keep Kotlin files in java source sets so the source set is not empty when
      // running unit tests which would prevent the tests from running in CI.
      java.includes.add("**/*.kt")
    }
    sourceSets.findByName("androidTest")?.apply {
      java.srcDirs("src/androidAndroidTest/kotlin")
      res.srcDirs("src/androidAndroidTest/res")
      assets.srcDirs("src/androidAndroidTest/assets")
      // Keep Kotlin files in java source sets so the source set is not empty when
      // running unit tests which would prevent the tests from running in CI.
      java.includes.add("**/*.kt")
    }
  }
}

afterEvaluate {
  configureSourceSets()
}
