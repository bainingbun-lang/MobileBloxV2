#pragma once

#include <EGL/egl.h>
#include <GLES3/gl3.h>

namespace renderer {
    // Initialize renderer (called from JNI_OnLoad)
    auto init() -> void;
    
    // Render frame (called from UI manager)
    auto render() -> void;
    
    // Set viewport size
    auto set_viewport(int width, int height) -> void;
    
    // Get EGL display
    auto get_egl_display() -> EGLDisplay;
    
    // Get EGL surface
    auto get_egl_surface() -> EGLSurface;
    
    // Get EGL context
    auto get_egl_context() -> EGLContext;
}