# Version history

# Setup

git update submodule

update MOLTENVK_PATH GLSLC in the CMakeLists.txt so that glslc can be found

build in Android Studio

# Introduction

It is possible to show a Vulkan rendered surface, in a part of the screen, from Kotlin or Swift, and not just in pure C/C++, or Objective-C.

Vulkan can improve the frame rate, and save the batteries.

kotlin:

This repository contains a buildable working project for kotlin.

It is highly inspired from the official Vulkan samples. The dynamic linking of vulkan was removed since it was buggy.
The android sample from LunarG uses a native activity. Instead, we use a SurfaceView in kotlin to display the result.
https://github.com/LunarG/VulkanSamples.git

If it does not work, follow carefully the LunarG tutorial to fix things, re-generate the shaders, etc.

The Khronos code sample was not used much (only small code copied).
Do not confuse the LunarG sample and the KhronosGroup one.
https://github.com/KhronosGroup/Vulkan-Samples

swift:

The sample below uses MoltenVK to use Vulkan using metal.

The sample assumes https://github.com/LunarG/VulkanSamples.git is in a parent folder since the same C++ files are used
using Objective-C++.

The sample shows a Vulkan graphics inside a viewController.

Using controller containers, one can have a small box inside the app, in swift.

https://github.com/KhronosGroup/MoltenVK

location of the Objective-C++ code:

MoltenVK/Demos/LunarG-VulkanSamples/API-Samples/iOS/DemoViewController.mm

# Sources

the android tutorial
https://developer.android.com/ndk/guides/graphics/getting-started

the vulkan code sample
git clone --recursive https://github.com/LunarG/VulkanSamples.git

the idea to use an android SurfaceView
https://stackoverflow.com/questions/45157950/can-we-use-vulkan-with-java-activity-on-android-platform

# Links

main idea to use vulkan from kotlin/java

https://stackoverflow.com/questions/45157950/can-we-use-vulkan-with-java-activity-on-android-platform


an example using jni

https://github.com/android/ndk-samples/tree/android-mk/hello-jni


some ideas about how to compile shaders in android studio

https://developer.android.com/ndk/guides/graphics/shader-compilers

https://developer.android.com/codelabs/android-studio-cmake#0


we do not use the dynamic loading because it reaquires lots of unstable changes
as illustraded below
https://marcelbraghetto.github.io/a-simple-triangle/2019/06/16/part-17/


about the vulkan pipeline

https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Window_surface


about the concepts of swapchain and command buffers and queues

https://software.intel.com/content/www/us/en/develop/articles/api-without-secrets-introduction-to-vulkan-part-1.html

https://software.intel.com/content/www/us/en/develop/articles/api-without-secrets-introduction-to-vulkan-part-2.html



# Licenses

Third party libraries are licensed. The have licenses mentioned directly in their files.

Apache Licenses:
* Copyright (C) 2015-2016 Valve Corporation
* Copyright (C) 2015-2016 LunarG, Inc.


This seems like an MIT-license, but it has no name:

/// OpenGL Mathematics (glm.g-truc.net)

/// Copyright (c) 2005 - 2014 G-Truc Creation (www.g-truc.net)


This repository itself is unlicensed for simplicity.

If you want to show credits, you can mention the following:

Special thanks to:

--> Thierry Vilmart, for making the whole idea work, showing a vulkan rendered surface in kotlin

--> Vlad, for the theoretical idea to use an android SurfaceView
https://stackoverflow.com/questions/45157950/can-we-use-vulkan-with-java-activity-on-android-platform

--> LunarG for the Vulkan code samples:
https://github.com/LunarG/VulkanSamples.git

# More info about the Android Rendering

On Android, VkAndroidSurfaceCreateInfoKHR attaches the ANativeWindow from the SurfaceView.

The next frame callback is faster with the C version of the Choreographer than in kotlin,
see the separate commits. The difference is visible with the eye. the performance improvement
comes from Garbage collection and bridge kotlin/C calls that are avoided in the C version.

For the next frame callback, from API level 33, the Choreographer lets the developer chose among multiple timelines
the presentation time depending on the deadline the developer can meet.

https://developer.android.com/ndk/reference/group/choreographer#achoreographer_registerrefreshratecallback
https://developer.android.com/about/versions/13/features#choreographer
https://android.googlesource.com/platform/frameworks/native/+/master/include/android/choreographer.h

https://lwn.net/Articles/809545/
https://source.android.com/docs/core/graphics/implement-vsync
https://source.android.com/docs/core/graphics/arch-vulkan
https://android-developers.googleblog.com/2020/04/high-refresh-rate-rendering-on-android.html
