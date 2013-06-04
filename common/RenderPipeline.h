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
#include <OpenGLES/ES2/gl.h>
#else // linux
#include <GL/glew.h>
#include <stdio.h>
#endif

class RenderPipeline {
public:

    RenderPipeline();
    void ClearBuffers();
    uint8_t * RayTracePixel(float x, float y, bool geometry);
    
    GLuint frameBuffer;

    GLuint colorTexture; // R, G, B, UNUSED (specular)
    GLuint depthBuffer;
    
    GLuint geometryDepthBuffer;
    GLuint geometryTexture; // NX_MV, NY_MV, NZ_MV, Depth_MVP
    unsigned int geometryTextureWidth;
    unsigned int geometryTextureHeight;

    GLuint geometryShader;
    GLuint halfGeometryShader;
    
};

#endif // __nativeGraphics__RenderPipeline__
