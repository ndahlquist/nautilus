//  RenderPipeline.cpp
//  nativeGraphics

#include "RenderPipeline.h"

#include "common.h"
#include "glsl_helper.h"
#include "log.h"
#include "cmath"

RenderPipeline::RenderPipeline() {
    
    defaultFrameBuffer = 0;
    
    geometryShader = createShaderProgram((char *)loadResource("standard_v.glsl"), (char *)loadResource("geometry_f.glsl"));
    halfGeometryShader = createShaderProgram((char *)loadResource("standard_v.glsl"), (char *)loadResource("geometry_far_f.glsl"));
    
    glViewport(0, 0, displayWidth, displayHeight);
    checkGlError("glViewport");
    
    // Allocate frame buffer
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    
    // Allocate depth buffer
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, displayWidth, displayHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    
    // Allocate albedo texture to render to.
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, displayWidth, displayHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    checkGlError("colorTexture");
    
    geometryTextureWidth = displayWidth;
    geometryTextureHeight = displayHeight;
    
    // Allocate geometry depth buffer
    glGenRenderbuffers(1, &geometryDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, geometryDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, geometryTextureWidth, geometryTextureHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, geometryDepthBuffer);
    
    // Allocate geometry texture to render to.
    glGenTextures(1, &geometryTexture);
    glBindTexture(GL_TEXTURE_2D, geometryTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, geometryTextureWidth, geometryTextureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    checkGlError("geometryTexture");
    
    glBindTexture(GL_TEXTURE_2D, 0);       
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

inline int clamp(int x, int a, int b) {
    return x < a ? a : (x > b ? b : x);
}

uint8_t * RenderPipeline::RayTracePixel(float x, float y, bool geometry) {
    
    int xpixel, ypixel;
    if(geometry) {
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, geometryTexture, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, geometryDepthBuffer);
        xpixel = clamp((int) floor(x * pipeline->geometryTextureWidth), 0, pipeline->geometryTextureWidth - 1);
        ypixel = clamp((int) floor(y * pipeline->geometryTextureHeight), 0, pipeline->geometryTextureHeight - 1);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
        xpixel = clamp((int) floor(x * displayWidth), 0, displayWidth - 1);
        ypixel = clamp((int) floor(y * displayHeight), 0, displayHeight - 1);
    }

    uint8_t * data = new uint8_t[4];
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    checkGlError("glPixelStorei");
    glReadPixels(xpixel, ypixel, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
    checkGlError("glReadPixels");
    
    return data;

}

void RenderPipeline::ClearBuffers() {
    
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    
    glClearColor(0., 0., 0., 1.);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");
    
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, geometryTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, geometryDepthBuffer);
    
    glClear(GL_DEPTH_BUFFER_BIT);
    checkGlError("glClear");
    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer);
    
    glClear(GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
