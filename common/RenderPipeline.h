//  RenderPipeline.h
//  nativeGraphics

#ifndef __nativeGraphics__RenderPipeline__
#define __nativeGraphics__RenderPipeline__

#ifdef ANDROID_NDK
#include "importgl.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <jni.h>
//#elif __APPLE__
//#include <stdlib.h>
//#include <stdint.h>
//#include <OpenGLES/ES2/gl.h>
#else // linux
#include <GL/glew.h>
#include <stdio.h>
#endif

#include "Eigen/Core"

class RenderPipeline {
public:

    RenderPipeline();
    void ClearBuffers();
    uint8_t getDepth(float x, float y);
    Eigen::Vector3f getNormal(float x, float y, Eigen::Matrix4f mvp);

    GLuint frameBuffer;

    GLuint gBuffer; // R, G, B, Depth_MVP
    GLuint depthBuffer;

};

#endif // __nativeGraphics__RenderPipeline__
