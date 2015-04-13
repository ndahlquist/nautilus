// glsl_helper.h
// nativeGraphics
// Compiles and links GLSL shader programs

#ifndef __nativeGraphics__glsl_helper__
#define __nativeGraphics__glsl_helper__

#include "graphics_header.h"

GLuint createShaderProgram(const char* pVertexSource = NULL, const char* pFragmentSource = NULL);

#endif // __nativeGraphics__glsl_helper__
