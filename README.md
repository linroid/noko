# noko
[WIP]Kotlin MPP binding to Node.js

### Example
```kotlin
val node = Node(keepAlive = true).also {
        it.start("-i")
      }
```

### Features
 - Automaticlly release JS object
 - Work with Jetpack Compose out-of-box
 - No data serialize/deserialize

### Supported Platforms

|          | arm64  | x64  |
|   ---    | --- | ---|
|  Windows |  ✔️|  ✔️ |
|  Linux   |  ✔️ | ✔️  |
|  macOS   | ✔️  | ✔️  |
|  Android |  ✔️ |  ✔️ |
|  iOS     |  N/A | N/A  |
|  JS      |  TBD  | TBD  |
