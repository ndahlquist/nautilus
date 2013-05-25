// glsl_helper.h
// nativeGraphics
// Compiles and links GLSL shader programs

#ifndef __nativeGraphics__glsl_helper__
#define __nativeGraphics__glsl_helper__

#ifdef ANDROID_NDK
    #include "importgl.h"
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
    #include <jni.h>
#elif __APPLE__
    #include <stdlib.h>
    #include <OpenGLES/ES2/gl.h>
#else // linux
    #include <GL/glew.h>
    #include <stdio.h>
#endif

GLuint createShaderProgram(const char* pVertexSource = NULL, const char* pFragmentSource = NULL);

#endif // __nativeGraphics__glsl_helper__
