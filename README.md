
# Setup

clone MoltenVK
./fetchDependency --ios
make ios
change the MOLTENVK_PATH in the project settings

## glslc to compile shaders

glslc is provided by the Vulkan sdk in macOS/bin/glsc

# Summary

The application must be in C and not in C++. iOS has only the last iOS version for C++.
Moreover, for low level stuff, C is suitted to have clear structures.
Moreover, C has better inter-operability with the operating system.

An XCFramework is highly recommended in the doc to simplify the bundling

the 2 files cube2.h and cube.c must be kept identical
A duplication is required because iOS can only build using the header cube2.h

how to log:
//__android_log_print(ANDROID_LOG_INFO, "LOG", "%lf - &lf - %lf", demo->spin_angle, elapsedTimeS, movedAngle);
//NSLog(@"%lf", movedAngle);

# setup

update the path in project settings / build settings / user defined
MOLTENVK_PATH

for mor performance, disable GPU tracing in Edit scheme Release as said in the doc

# Links

https://github.com/KhronosGroup/MoltenVK/
https://github.com/KhronosGroup/MoltenVK/blob/master/Docs/MoltenVK_Runtime_UserGuide.md
https://moltengl.com/moltenvk/
https://vulkan-tutorial.com/Drawing_a_triangle/Drawing/Rendering_and_presentation
MoltenVK uses Vulkan 1.1.198
https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#commandbuffers

for the camera in SwiftUI
https://www.raywenderlich.com/26244793-building-a-camera-app-with-swiftui-and-combine

# Summary of what worked to run the demo

follow the instructions and add the include path using ${MOLTENVK_PATH}/include

set the path variable in project settings / build settings / user defined
MOLTENVK_PATH

./fetchdependencies --ios
make ios
remove the file in gtx, the quaternion.inl from build phases copy Bundle Resources

open the packaging .xcproj and build the iOS only scheme on any iOS device
or better use the make


This was changed in cube.c compared to the demo
// we do not want opaque
compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
VkSwapchainCreateInfoKHR swapchain_ci

# Nice tools

## obj loader
https://github.com/syoyo/tinyobjloader-c
Geometry can be mad in the software called Blender, then exported as a .obj

##OOP in C
https://www.codementor.io/@michaelsafyan/object-oriented-programming-in-c-du1081gw2

# PNG on iOS
The image is grey because gamma and cHRM are used, shown on 32 bits via the command xxd on the png.

## draw text
with textures or with path drawing
https://community.khronos.org/t/drawing-graphical-text-in-vulkan/105462

# Blender to make a .obj
on the cube, add a bevel transform to make stairs borders
use remesh modifier to make round corners
use multiple ones

use the knife to draw a rectangle on a surface with more vertices

n to show the transform panel
how to make exact .obj export
uncheck all except triangulate

add the apply transform to get the smooth back and with less decimals Y is still fine

# Links

Vulkan synchronization examples
https://github.com/cforfang/Vulkan-Tools/wiki/Synchronization-Examples
https://www.lunarg.com/wp-content/uploads/2021/08/Vulkan-Synchronization-SIGGRAPH-2021.pdf

# Debug with validation layers - IMPORTANT

This is important to check anomalies in the use of Vulkan

The best to check validation is to run on macOS (not iOS simulator).
It will ahve all the needed extensions. For this to work you need to install the macOS vulkan SDK .dmg file in your home folder
The vulkan loader with the validation layers in the SDK does not work for iOS, it only works for macOS.

On android, validation does not wortk in the Emulator.
It works on a real device but it will lack an extension VK_EXT_debug_utils

https://developer.android.com/ndk/guides/graphics/validation-layer
just copy the zip content in the jniLib folder
And put the flag vulkanDSL->validate = true

then adb logcat filtered on VALIDATION, or breakpoints in the callback function debug_messenger_callback
will show the misuses of Vulkan.
In the vulkan tutorial, there is even a test, removing DestroyDebugUtilsMessengerEXT in the cleanup

