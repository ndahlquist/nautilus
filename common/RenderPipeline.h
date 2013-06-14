//  RenderPipeline.h
//  nativeGraphics

#ifndef __nativeGraphics__RenderPipeline__
#define __nativeGraphics__RenderPipeline__

#ifdef ANDROID_NDK
#include "importgl.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <jni.h>
#elif __APPLE__
#include <stdlib.h>
#include <stdint.h>
#include <OpenGLES/ES2/gl.h>
#else // linux
#include <GL/glew.h>
#include <stdio.h>
#endif

class RenderPipeline {
public:

    RenderPipeline();
    void ClearBuffers();
    //uint8_t * RayTracePixel(float x, float y, bool geometry);
    
    GLuint frameBuffer;

    GLuint gBuffer; // R, G, B, Depth_MVP
    GLuint depthBuffer;
    
};

#endif // __nativeGraphics__RenderPipeline__
