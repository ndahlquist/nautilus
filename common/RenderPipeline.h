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
    void saveShadowMatrices();
    
    GLfloat* mv_shadow;
    GLfloat* mv_inverse_shadow;
    GLfloat* mvp_shadow;
    
    GLuint frameBuffer;
    GLuint depthBuffer;

    GLuint colorTexture; // R, G, B, UNUSED (specular)
    GLuint geometryTexture; // NX_MV, NY_MV, NZ_MV, Depth_MVP

    GLuint geometryShader;
    
    GLuint shadowFrameBuffer;
    GLuint shadowDepthBuffer;
    GLuint shadowTexture;
    
};

#endif // __nativeGraphics__RenderPipeline__
