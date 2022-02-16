plugins {
  kotlin("multiplatform") version "1.6.10"
  kotlin("plugin.serialization") version "1.6.10"
  id("com.android.library")
}

group = "com.linroid.noko"
version = "1.0-SNAPSHOT"

val coroutinesVersion = "1.6.0"

repositories {
  google()
  mavenCentral()
}

android {
  compileOptions {
    sourceCompatibility = JavaVersion.VERSION_1_8
    targetCompatibility = JavaVersion.VERSION_1_8
  }
  defaultConfig {
    testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    consumerProguardFiles(file("consumer-rules.pro"))
    externalNativeBuild {
      cmake {
        cppFlags += "-std=c++11 -fexceptions"
        arguments += setOf("-DANDROID_ARM_MODE=arm", "-DANDROID_STL=c++_shared")
      }
    }
  }
  externalNativeBuild {
    cmake {
      path = file("src/commonMain/cpp/CMakeLists.txt")
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

kotlin {
  android {}
  jvm {
    compilations.all {
      kotlinOptions.jvmTarget = "1.8"
      kotlinOptions.noJdk = true
    }
    testRuns["test"].executionTask.configure {
      useJUnit()
    }
  }
  jvm("desktop") {
    compilations.all {
      kotlinOptions.jvmTarget = "1.8"
    }
    testRuns["test"].executionTask.configure {
      useJUnit()
    }
  }

  sourceSets {
    val commonMain by getting {
      dependencies {
        implementation(kotlin("test"))
        implementation(kotlin("stdlib"))
        implementation("org.jetbrains.kotlinx:kotlinx-serialization-json:1.3.2")
        implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core:$coroutinesVersion")
      }
    }
    val commonTest by getting
    val jvmMain by getting {
      dependsOn(commonMain)
      dependencies {
        implementation(kotlin("stdlib"))
        implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core-jvm:$coroutinesVersion")
      }
    }
    val jvmTest by getting {
      dependsOn(jvmMain)
      dependsOn(commonTest)
    }

    val desktopMain by getting {
      dependsOn(commonMain)
      dependencies {
        implementation(kotlin("stdlib"))
        implementation("org.jetbrains.kotlinx:kotlinx-coroutines-core-jvm:$coroutinesVersion")
      }
    }
    val desktopTest by getting {
      dependsOn(desktopMain)
    }
    val androidMain by getting {
      dependsOn(commonMain)
      dependencies {
        implementation("org.jetbrains.kotlinx:kotlinx-coroutines-android:$coroutinesVersion")
      }
    }
    val androidTest by getting {
      dependsOn(androidMain)
      dependencies {
        implementation("junit:junit:4.13.2")
      }
    }
  }
}

android {
  compileSdk = 31
  sourceSets["main"].manifest.srcFile("src/androidMain/AndroidManifest.xml")
  defaultConfig {
    minSdk = 24
    targetSdk = 31
  }
}