//  RenderLight.cpp
//  nativeGraphics

#include "RenderLight.h"
#include "glsl_helper.h"
#include "transform.h"
#include "common.h"
#include "log.h"

RenderLight::RenderLight(const char *objFilename, const char *vertexShaderFilename, const char *fragmentShaderFilename) : RenderObject(objFilename, vertexShaderFilename, fragmentShaderFilename, false) {
   color[0] = 1.0f;
   color[1] = 1.0f;
   color[2] = 1.0f;
   brightness = 1000.0f;
}

void RenderLight::Render() {

    if(!pipeline) {
        LOGE("RenderPipeline inaccessible.");
        exit(0);
    }
    
    glUseProgram(colorShader);
    checkGlError("glUseProgram");
    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer);
    glViewport(0, 0, displayWidth, displayHeight);
    
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE);
    glEnable(GL_DITHER);
    
    // Pass fragment size
    GLuint u_FragWidth = glGetUniformLocation(colorShader, "u_FragWidth");
    glUniform1i(u_FragWidth, displayWidth);
    GLuint u_FragHeight = glGetUniformLocation(colorShader, "u_FragHeight");
    glUniform1i(u_FragHeight, displayHeight);   
    
    // Pass color
    GLuint colorUniform = glGetUniformLocation(colorShader, "u_Color");
    glUniform3f(colorUniform, color[0], color[1], color[2]);
    
    // Pass brightness
    GLuint brightnessUniform = glGetUniformLocation(colorShader, "u_Brightness");
    glUniform1f(brightnessUniform, brightness);
    
    // Pass matrices
    GLfloat* mv_Matrix = (GLfloat*)mvMatrix();
    GLfloat* mvp_Matrix = (GLfloat*)mvpMatrix();
    GLfloat* pT_Matrix = (GLfloat*)pInverseMatrix();
    glUniformMatrix4fv(gmvMatrixHandle, 1, GL_FALSE, mv_Matrix);
    glUniformMatrix4fv(gmvpMatrixHandle, 1, GL_FALSE, mvp_Matrix);
    GLuint gpT_MatrixHandle = glGetUniformLocation(colorShader, "u_pT_Matrix");
    glUniformMatrix4fv(gpT_MatrixHandle, 1, GL_FALSE, pT_Matrix);
    checkGlError("glUniformMatrix4fv");
    delete[] mv_Matrix;
    delete[] mvp_Matrix;
    delete[] pT_Matrix;
    
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    
    // Pass vertices
    glEnableVertexAttribArray(gvPositionHandle);
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid*) 0);
    checkGlError("gvPositionHandle");
    
    // Pass g buffer
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pipeline->gBuffer);
    GLuint colorTextureUniform = glGetUniformLocation(colorShader, "u_gBuffer");
    glUniform1i(colorTextureUniform, 0);
    checkGlError("pass gBuffer");
    
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
    checkGlError("glDrawArrays");
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
}
