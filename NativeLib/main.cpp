#include <thread>
#include <inttypes.h>
#include <android/native_window.h>
#include <EGL/egl.h>

#include "renderer/renderer.hpp"
#include "roblox/hooks.hpp"
#include "roblox/update.hpp"
#include "utils/utils.hpp"
#include "ui/ui_manager.hpp"

static mobileblox::ui::UIManager g_ui_manager;
static ANativeWindow* g_native_window = nullptr;
static EGLDisplay g_egl_display = EGL_NO_DISPLAY;

void sig_handler(int sig) {
    LOGE(" [sig_handler] Caught a crash, signal: {%d}", sig);
    utils::backtrace::logcat(30);
    exit(EXIT_FAILURE);
}

// Called from Java/Kotlin when surface is created
extern "C" JNIEXPORT void JNICALL
Java_com_mobileblox_v2_MainActivity_onSurfaceCreated(JNIEnv* env, jobject thiz, jobject surface) {
    if (surface) {
        g_native_window = ANativeWindow_fromSurface(env, surface);
        LOGI(" [Surface] Created: %p", g_native_window);
    }
}

// Called from Java/Kotlin when surface is destroyed
extern "C" JNIEXPORT void JNICALL
Java_com_mobileblox_v2_MainActivity_onSurfaceDestroyed(JNIEnv* env, jobject thiz) {
    if (g_native_window) {
        ANativeWindow_release(g_native_window);
        g_native_window = nullptr;
        LOGI(" [Surface] Destroyed");
    }
}

// Called from Java/Kotlin for each frame
extern "C" JNIEXPORT void JNICALL
Java_com_mobileblox_v2_MainActivity_onDrawFrame(JNIEnv* env, jobject thiz) {
    if (!g_native_window) return;
    
    // Initialize UI on first frame
    static bool ui_initialized = false;
    if (!ui_initialized) {
        // Get EGL display
        g_egl_display = eglGetCurrentDisplay();
        if (g_ui_manager.Initialize(g_native_window, (void*)g_egl_display)) {
            ui_initialized = true;
            LOGI(" [UI] Initialized successfully");
        } else {
            LOGE(" [UI] Failed to initialize");
        }
    }
    
    if (ui_initialized) {
        g_ui_manager.NewFrame();
        g_ui_manager.Render();
        g_ui_manager.EndFrame();
    }
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI(" [JNIUOL] JVM: %p", vm);
    
    if (vm->GetEnv(reinterpret_cast<void**>(&utils::JNI::env), JNI_VERSION_1_6) != JNI_OK) {
        LOGE(" [JNIUOL] Failed to get JNIEnv.");
        return -1;
    }
    
    LOGI(" [JNIUOL] JNIEnv: %p", utils::JNI::env);
    
    // ==================================== //
    
    signal(SIGSEGV, sig_handler);
    
    LOGI(" [JNIUOL] Waiting for libroblox.so... ");
    
    utils::memory::wait_for_lib("libroblox.so");
    LOGI(" [JNIUOL] libroblox.so Base Address -> 0x%" PRIXPTR " | Initiating Exploit. ", utils::memory::find_lib("libroblox.so"));
    
    roblox::functions::init();
    roblox::hooks::init();
    renderer::init();
    
    LOGI(" [JNIUOL] Initiated Exploit.");
    
    return JNI_VERSION_1_6;
}