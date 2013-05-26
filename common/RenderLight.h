//  RenderLight.h
//  nativeGraphics

#ifndef __nativeGraphics__RenderLight__
#define __nativeGraphics__RenderLight__

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

#include <vector>

#include "RenderObject.h"

using namespace std;

class RenderLight : public RenderObject {
public:
    RenderLight(const char *objFile, const char *vertexShaderFile, const char *fragmentShaderFile);
    void RenderFrame();
    
    float brightness[3]; // R, G, B
};


#endif // __nativeGraphics__RenderLight__
