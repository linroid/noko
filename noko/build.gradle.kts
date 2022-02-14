plugins {
  kotlin("multiplatform") version "1.6.10"
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
    sourceCompatibility = JavaVersion.VERSION_16
    targetCompatibility = JavaVersion.VERSION_16
  }
  defaultConfig {
    testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    consumerProguardFiles(file("consumer-rules.pro"))
    externalNativeBuild {
      // cmake {
      //     cppFlags += "-std=c++11 -fexceptions"
      //     arguments += kotlin.collections.setOf("-DANDROID_ARM_MODE=arm", "-DANDROID_STL=c++_shared")
      // }
    }
  }
  externalNativeBuild {
    cmake {
      path = file("src/commonMain/cpp/CMakeLists.txt")
      version = "3.10.2"
    }
  }
  packagingOptions {
    // jniLibs {
    //   keepDebugSymbols += "**/*.so"
    // }
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
  sourceSets {
    val commonMain by getting {
      dependencies {
        implementation(kotlin("test"))
        implementation(kotlin("stdlib"))
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
    val jvmTest by getting
    val androidMain by getting {
      dependsOn(commonMain)
      dependencies {
        implementation("org.jetbrains.kotlinx:kotlinx-coroutines-android:$coroutinesVersion")
      }
    }
    val androidTest by getting {
      dependencies {
        implementation("junit:junit:4.13.2")
      }
    }
  }
}

android {
  compileSdkVersion(31)
  sourceSets["main"].manifest.srcFile("src/androidMain/AndroidManifest.xml")
  defaultConfig {
    minSdkVersion(24)
    targetSdkVersion(31)
  }
}