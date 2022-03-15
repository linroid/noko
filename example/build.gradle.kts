import org.jetbrains.compose.compose
import org.jetbrains.compose.desktop.application.dsl.TargetFormat

plugins {
  kotlin("multiplatform")
  id("org.jetbrains.compose")
  id("com.android.application")
}

group = "com.linroid.noko"
version = "1.0"

kotlin {
  android()
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
        api(compose.runtime)
        api(compose.foundation)
        api(compose.ui)
        api(compose.material)
        @OptIn(org.jetbrains.compose.ExperimentalComposeLibrary::class)
        api(compose.material3)
        api(project(":noko"))
      }
    }
    val commonTest by getting {
      dependencies {
        implementation(kotlin("test"))
      }
    }
    val androidMain by getting {
      dependencies {
        api(compose.preview)
        api(project(":noko"))
        api("androidx.appcompat:appcompat:1.4.1")
        api("androidx.core:core-ktx:1.7.0")
        api("androidx.compose.material:material:1.1.1")
        implementation("androidx.activity:activity-compose:1.4.0")
      }
    }
    val androidTest by getting {
      dependencies {
        implementation("junit:junit:4.13.2")
      }
    }
    val desktopMain by getting {
      dependencies {
        api(compose.preview)
        implementation(compose.desktop.currentOs)
      }
    }
    val desktopTest by getting
  }
}

android {
  compileSdk = 31
  sourceSets["main"].manifest.srcFile("src/androidMain/AndroidManifest.xml")
  defaultConfig {
    minSdk = 24
    targetSdk = 31
  }
  compileOptions {
    sourceCompatibility = JavaVersion.VERSION_1_8
    targetCompatibility = JavaVersion.VERSION_1_8
  }
}

compose.desktop {
  application {
    mainClass = "com.linroid.noko.example.MainKt"
    nativeDistributions {
      targetFormats(
        TargetFormat.Dmg, TargetFormat.Msi, TargetFormat.Deb
      )
      packageName = "Noko Example"
      packageVersion = "1.0.0"
    }
  }
}