# Notes

The image is grey because gamma and cHRM are used, shown on 32 bits via the command xxd on the png.

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

# Summary of what worked

follow the instructions and add the include path using ${MOLTENVK_PATH}/include

set the path variable in project settings / build settings / user defined
MOLTENVK_PATH

./fetchdependencies --ios
open the packaging .xcproj and build the iOS only scheme on any iOS device
or better use the make
make iossim iossim-debug ios ios-debug
then the Demos will work


This must be changed in cube.c
// we do not want opaque
compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
VkSwapchainCreateInfoKHR swapchain_ci

