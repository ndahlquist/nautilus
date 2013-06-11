//  HUD.cpp
//  nativeGraphics

#include "HUD.h"
#include "glsl_helper.h"
#include "transform.h"
#include "common.h"
#include "log.h"

HUD::HUD() : RenderObject("square.obj", "overlay_v.glsl", "overlay.glsl", false) { 
    healthbarTex = AddTexture("healthbar.png");
    healthbarBorderTex = AddTexture("healthbar_border.png");
    radarTex = AddTexture("radar.png");
}

GLuint HUD::AddTexture(const char *textureFilename) {
    // Load textures
    int width, height;
    GLubyte *imageData = (GLubyte *)loadResource(textureFilename, &width, &height);
    
    GLuint newTex = -1;
    glGenTextures(1, &newTex);
    glBindTexture(GL_TEXTURE_2D, newTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    //free(imageData); // TODO: Not allowed on Samsung Galaxy (not malloc'd).
    
    checkGlError("AddTexture");
    
    return newTex;
}

void HUD::RenderElement(GLuint textureHandle, float xdisp, float ydisp, float xscale, float yscale) {
    
    if(!pipeline) {
        LOGE("RenderPipeline inaccessible.");
        exit(0);
    }
    
    glUseProgram(colorShader);
    checkGlError("glUseProgram");
    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer);
    glViewport(0, 0, displayWidth, displayHeight);
    
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DITHER);
    
    GLuint u_displacement = glGetUniformLocation(colorShader, "u_displacement");
    glUniform2f(u_displacement, xdisp, ydisp);
    
    GLuint u_scale = glGetUniformLocation(colorShader, "u_scale");
    glUniform2f(u_scale, xscale, yscale); 
    
    // Pass texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glUniform1i(textureUniform, 0);
    checkGlError("texture");

    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    
    // Pass vertices
    gvPositionHandle = glGetAttribLocation(colorShader, "a_Position");
    glEnableVertexAttribArray(gvPositionHandle);
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid*) 0);
    checkGlError("gvPositionHandle");
    
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
    checkGlError("glDrawArrays");
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void HUD::Render(float health) {

    RenderElement(healthbarBorderTex, -.1f, .8f, .8f, .08f);
    RenderElement(healthbarTex, -.15f, .8f, .7f, .08f);
    RenderElement(radarTex, -.8f, -.7f, .1f, .2f);
}
