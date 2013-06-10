//  RenderCaustic.h
//  nativeGraphics

#ifndef __nativeGraphics__RenderCaustic__
#define __nativeGraphics__RenderCaustic__

#include <iostream>
#include <string>

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

#include "RenderObject.h"
#include "Timer.h"

using namespace std;

class RenderCaustic : public RenderObject {
public:
    RenderCaustic(const char *objFile, const char *vertexShaderFile, const char *fragmentShaderFile);
    void Render();
    
    Timer timer;
    GLuint frameBuffer;
    GLuint outputTexture;
};


#endif // __nativeGraphics__RenderCaustic__
