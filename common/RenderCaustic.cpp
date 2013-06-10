//  RenderCaustic.cpp
//  nativeGraphics

#include "RenderCaustic.h"
#include "glsl_helper.h"
#include "transform.h"
#include "common.h"
#include "log.h"

RenderCaustic::RenderCaustic(const char *objFilename, const char *vertexShaderFilename, const char *fragmentShaderFilename) : RenderObject(objFilename, vertexShaderFilename, fragmentShaderFilename, false) {
    timer.reset();
   
    // Allocate frame buffer
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
   
    // Allocate texture to render to
    glGenTextures(1, &outputTexture);
    glBindTexture(GL_TEXTURE_2D, outputTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    checkGlError("outputTexture");
}

void RenderCaustic::Render() {
    
    //////////////////////////////////
    // Render to frame buffer
    
    SetShader(colorShader);
    
    glViewport(0, 0, 64, 64);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTexture, 0);
    glClearColor(0., 0., 0., 1.);
    glClear(GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);
    checkGlError("glClear");
    
    // Pass time
    GLuint timeUniform = glGetUniformLocation(colorShader, "u_Time");
    if(timeUniform == -1) {
        LOGE("There seems to be some confusion here.");
    }
    glUniform1f(timeUniform, timer.getSeconds());
    
    RenderPass(0, 0, -1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0); // TODO: unbind other resources

}
