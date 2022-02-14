-keep class com.linroid.noko.js.BindJS
#-keepclasseswithmembers class com.linroid.dora.** {
#    @ocom.linroid.noko.js.BindJS *;
#}
#-keepnames class com.linroid.dora.** {
#    @com.linroid.noko.js.BindJS *;
#}
-keepclassmembers class ** {
    @com.linroid.noko.js.BindJS *;
}
-keep class com.linroid.noko.** { *; }
-keepnames class com.linroid.noko.** { *; }
