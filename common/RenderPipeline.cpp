//  RenderPipeline.cpp
//  nativeGraphics

#include "RenderPipeline.h"

#include "common.h"
#include "glsl_helper.h"
#include "log.h"
#include "cmath"

#include "Eigen/Eigenvalues"

using Eigen::Vector3f;

RenderPipeline::RenderPipeline() {
    
    defaultFrameBuffer = 0;
    
    // Allocate frame buffer
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    
    // Allocate depth buffer
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, displayWidth, displayHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    
    // Allocate albedo texture to render to.
    glGenTextures(1, &gBuffer);
    glBindTexture(GL_TEXTURE_2D, gBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, displayWidth, displayHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    checkGlError("gBuffer");
    
    glBindTexture(GL_TEXTURE_2D, 0);       
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

inline int clamp(int x, int a, int b) {
    return x < a ? a : (x > b ? b : x);
}

uint8_t RenderPipeline::getDepth(float x, float y) {

    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer, 0);
        
    int xpixel = clamp((int) floor(x * displayWidth), 0, displayWidth - 1);
    int ypixel = clamp((int) floor(y * displayHeight), 0, displayHeight - 1);
    
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    checkGlError("glPixelStorei");
    
    uint8_t data[4]; // TODO
    glReadPixels(xpixel, ypixel, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &data);
    checkGlError("glReadPixels");
    
    return data[3];
}

Eigen::Vector3f RenderPipeline::getNormal(float x, float y, Eigen::Matrix4f mvp) {

    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer, 0);
        
    int xpixel = clamp((int) floor(x * displayWidth), 0, displayWidth - 6);
    int ypixel = clamp((int) floor(y * displayHeight), 0, displayHeight - 6);
    
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    checkGlError("glPixelStorei");
    
    uint8_t data[4]; // TODO
    glReadPixels(xpixel, ypixel, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &data);
    checkGlError("glReadPixels");
    Eigen::Vector4f pos0 = mvp.inverse() * Eigen::Vector4f(2.0f * xpixel / (float) displayWidth - 1, 2.0f * ypixel / (float) displayHeight - 1, data[3] / 128.0f - 1.0f, 1.0);
    
    glReadPixels(xpixel + 5, ypixel, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &data);
    checkGlError("glReadPixels");
    Eigen::Vector4f pos1 = mvp.inverse() * Eigen::Vector4f(2.0f * (xpixel+5) / (float) displayWidth - 1, 2.0f * ypixel / (float) displayHeight - 1, data[3] / 128.0f - 1.0f, 1.0);
    
    glReadPixels(xpixel, ypixel + 5, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &data);
    checkGlError("glReadPixels");
    Eigen::Vector4f pos2 = mvp.inverse() * Eigen::Vector4f(2.0f * xpixel / (float) displayWidth - 1, 2.0f * (ypixel+5) / (float) displayHeight - 1, data[3] / 128.0f - 1.0f, 1.0);
    
    Eigen::Vector4f cross0t = pos0 / pos0(3) - pos1 / pos1(3);
    Vector3f cross0 = Vector3f(cross0t(0), cross0t(1), cross0t(2));
    
    Eigen::Vector4f cross1t = pos0 / pos0(3) - pos2 / pos2(3);
    Vector3f cross1 = Vector3f(cross1t(0), cross1t(1), cross1t(2));

    return cross0.cross(cross1).normalized();
}

void RenderPipeline::ClearBuffers() {
    
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gBuffer, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    glClearColor(0., 0., 0., 1.);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");
    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer);
    glClear(GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
