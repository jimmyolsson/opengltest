#pragma once
#include "common.h"

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#else
#include <glad/glad.h>
#endif

#define GL_CALL( x ) \
    { \
        x; \
        GLenum error = glGetError(); \
        if( error != GL_NO_ERROR ) { \
            g_logger_error("GL ERROR: %d,  %s\n", (int)error, #x ); \
            assert(false); \
        } \
    } 
