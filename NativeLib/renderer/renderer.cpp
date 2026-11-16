#include "renderer/renderer.hpp"
#include "utils/utils.hpp"
#include <EGL/egl.h>
#include <GLES3/gl3.h>

namespace {
    EGLDisplay g_egl_display = EGL_NO_DISPLAY;
    EGLSurface g_egl_surface = EGL_NO_SURFACE;
    EGLContext g_egl_context = EGL_NO_CONTEXT;
    int g_width = 0;
    int g_height = 0;
}

namespace renderer {
    auto init() -> void {
        // Get current EGL context (created by Android)
        g_egl_display = eglGetCurrentDisplay();
        g_egl_surface = eglGetCurrentSurface(EGL_DRAW);
        g_egl_context = eglGetCurrentContext();
        
        if (g_egl_display == EGL_NO_DISPLAY) {
            LOGE("[Renderer] No EGL display available");
            return;
        }
        
        // Query surface dimensions
        EGLint width, height;
        eglQuerySurface(g_egl_display, g_egl_surface, EGL_WIDTH, &width);
        eglQuerySurface(g_egl_display, g_egl_surface, EGL_HEIGHT, &height);
        g_width = width;
        g_height = height;
        
        LOGI("[Renderer] Initialized: %dx%d", g_width, g_height);
        
        // Setup OpenGL state
        glViewport(0, 0, g_width, g_height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
    }
    
    auto render() -> void {
        if (g_egl_display == EGL_NO_DISPLAY) return;
        
        // Clear screen
        glViewport(0, 0, g_width, g_height);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // ImGui rendering is handled by UIManager
    }
    
    auto set_viewport(int width, int height) -> void {
        g_width = width;
        g_height = height;
        if (g_egl_display != EGL_NO_DISPLAY) {
            glViewport(0, 0, width, height);
        }
    }
    
    auto get_egl_display() -> EGLDisplay {
        return g_egl_display;
    }
    
    auto get_egl_surface() -> EGLSurface {
        return g_egl_surface;
    }
    
    auto get_egl_context() -> EGLContext {
        return g_egl_context;
    }
}