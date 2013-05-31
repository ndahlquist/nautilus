//  RenderPipeline.cpp
//  nativeGraphics

#include "RenderPipeline.h"

#include "common.h"
#include "transform.h"
#include "glsl_helper.h"
#include "log.h"

RenderPipeline::RenderPipeline() {
    
    defaultFrameBuffer = 0;
    
    geometryShader = createShaderProgram((char *)resourceCallback("standard_v.glsl"), (char *)resourceCallback("geometry_f.glsl"));
    
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
    
    // Allocate normal texture to render to.
    glGenTextures(1, &geometryTexture);
    glBindTexture(GL_TEXTURE_2D, geometryTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, displayWidth, displayHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    checkGlError("geometryTexture");
    
    // Allocate frame buffer
    glGenFramebuffers(1, &shadowFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFrameBuffer);
    
    // Allocate depth buffer
    glGenRenderbuffers(1, &shadowDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, shadowDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, displayWidth, displayHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, shadowDepthBuffer);
    
    // Allocate albedo texture to render to.
    glGenTextures(1, &shadowTexture);
    glBindTexture(GL_TEXTURE_2D, shadowTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, displayWidth, displayHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    checkGlError("colorTexture");
    
    glBindTexture(GL_TEXTURE_2D, 0);       
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderPipeline::saveShadowMatrices() {
    mv_shadow = (GLfloat*)mvMatrix(); // TODO: delete
    mv_inverse_shadow = (GLfloat*)mvInverseMatrix();
    mvp_shadow = (GLfloat*)mvpMatrix();
}

void RenderPipeline::ClearBuffers() {
    
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    
    glClearColor(0., 0., 0., 0.);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");
    
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, geometryTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    
    glClear(GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");
    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer);
    
    glClear(GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");
    
    glBindFramebuffer(GL_FRAMEBUFFER, pipeline->shadowFrameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pipeline->shadowTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pipeline->shadowDepthBuffer);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");
    
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
