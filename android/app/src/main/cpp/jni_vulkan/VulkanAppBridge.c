#include "VulkanAppBridge.h"

#include <android/log.h>
#include <android/native_window_jni.h>
#include <android/asset_manager_jni.h>

extern void createSurface(ANativeWindow* window);
extern void demoDestroy();
extern void drawFrame();
extern void setSize(int32_t width, int32_t height);

// sources for the idea of using an android SurfaceView
// https://stackoverflow.com/questions/45157950/can-we-use-vulkan-with-java-activity-on-android-platform
// https://vulkan-tutorial.com/Drawing_a_triangle/Presentation/Window_surface

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    __android_log_print(ANDROID_LOG_VERBOSE, "VulkanAppBridge", "JNI_OnLoad");
    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL
Java_com_thierry_android_1surface_1view_1with_1vulkan_VulkanAppBridge_nativeCreate(JNIEnv *env, jobject vulkanAppBridge,
                                                     jobject surface, jobject pAssetManager) {
    __android_log_print(ANDROID_LOG_DEBUG, "mc-native-VulkanAppBridge", "create");
    ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
    __android_log_print(ANDROID_LOG_VERBOSE, "AndroidGraphicsApplication", "createSurface");
    createSurface(window);
}

JNIEXPORT void JNICALL
Java_com_thierry_android_1surface_1view_1with_1vulkan_VulkanAppBridge_nativeDestroy(JNIEnv *env, jobject vulkanAppBridge) {
    __android_log_print(ANDROID_LOG_DEBUG, "mc-native-VulkanAppBridge", "destroy");
    demoDestroy();
}

JNIEXPORT void JNICALL
Java_com_thierry_android_1surface_1view_1with_1vulkan_VulkanAppBridge_nativeResize(JNIEnv *env, jobject vulkanAppBridge, jint width, jint height) {
    __android_log_print(ANDROID_LOG_DEBUG, "mc-native-VulkanAppBridge", "resize: %dx%d", width, height);
    setSize(width, height);
}

JNIEXPORT void JNICALL
Java_com_thierry_android_1surface_1view_1with_1vulkan_VulkanAppBridge_nativeDraw(JNIEnv *env, jobject vulkanAppBridge, jdouble elapsedTimeS) {
    __android_log_print(ANDROID_LOG_DEBUG, "mc-native-VulkanAppBridge", "draw");
    drawFrame(elapsedTimeS);
}
