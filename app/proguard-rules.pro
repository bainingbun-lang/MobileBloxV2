# MobileBlox V2 ProGuard Rules

# Keep native library loading
-keep class com.mobileblox.v2.MobileBloxApplication { *; }
-keep class com.mobileblox.v2.MainActivity { *; }

# Keep Kotlin coroutines
-keep class kotlinx.coroutines.** { *; }

# Keep Gson for JSON serialization
-keep class com.google.gson.** { *; }
-keepattributes Signature, *Annotation*

# Keep Timber logging
-keep class timber.log.** { *; }

# Keep Material3 components
-keep class com.google.android.material.** { *; }

# Native JNI methods (if any Java/Kotlin JNI wrappers)
# -keepclasseswithmembernames class * {
#     native <methods>;
# }

# Don't obfuscate resource IDs
-keepclassmembers class **.R$* {
    public static <fields>;
}

# Keep parcelable creators
-keepclassmembers class * implements android.os.Parcelable {
    public static final android.os.Parcelable$Creator *;
}

# Keep enum values
-keepclassmembers enum * {
    public static **[] values();
    public static ** valueOf(java.lang.String);
}

# Optimization
-optimizations !code/simplification/arithmetic,!field/*,!class/merging/*
-optimizationpasses 5