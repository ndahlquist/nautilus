//  RenderLight.cpp
//  nativeGraphics

#include "RenderLight.h"
#include "glsl_helper.h"
#include "transform.h"
#include "common.h"
#include "log.h"

RenderLight::RenderLight(const char *objFilename, const char *vertexShaderFilename, const char *fragmentShaderFilename) : RenderObject(objFilename, vertexShaderFilename, fragmentShaderFilename) {
   brightness[0] = 100.0f;
   brightness[1] = 100.0f;
   brightness[2] = 100.0f;
}

void RenderLight::RenderFrame() {

    if(!pipeline) {
        LOGE("RenderPipeline inaccessible.");
        exit(0);
    }
    
    glUseProgram(colorShader);
    checkGlError("glUseProgram");
    
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer);
    
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
    
    // Pass brightness
    GLuint brightnessUniform = glGetUniformLocation(colorShader, "u_Brightness");
    glUniform3f(brightnessUniform, brightness[0], brightness[1], brightness[2]);
    
    // Pass matrices
    GLfloat* mv_Matrix = (GLfloat*)mvMatrix();
    glUniformMatrix4fv(gmvMatrixHandle, 1, GL_FALSE, mv_Matrix);
    delete mv_Matrix;
    
    GLfloat* mvp_Matrix = (GLfloat*)mvpMatrix();
    glUniformMatrix4fv(gmvpMatrixHandle, 1, GL_FALSE, mvp_Matrix);
    delete mvp_Matrix;
    
    GLfloat* p_inverse_Matrix = (GLfloat*)pInverseMatrix();
    GLuint u_p_inverseHandle = glGetUniformLocation(colorShader, "u_p_inverse");
    glUniformMatrix4fv(u_p_inverseHandle, 1, GL_FALSE, p_inverse_Matrix);
    delete p_inverse_Matrix;
    
    GLfloat* mv_inverse_Matrix = (GLfloat*)mvInverseMatrix();
    GLuint u_mv_inverseHandle = glGetUniformLocation(colorShader, "u_mv_inverse");
    glUniformMatrix4fv(u_mv_inverseHandle, 1, GL_FALSE, mv_inverse_Matrix );
    delete mv_inverse_Matrix;
    checkGlError("glUniformMatrix4fv");
    
    // Pass shadow matrices
    GLuint u_mv_lightHandle = glGetUniformLocation(colorShader, "u_mv_light");
    glUniformMatrix4fv(u_mv_lightHandle, 1, GL_FALSE, pipeline->mv_shadow);
    GLuint u_mvp_lightHandle = glGetUniformLocation(colorShader, "u_mvp_light");
    glUniformMatrix4fv(u_mvp_lightHandle, 1, GL_FALSE, pipeline->mvp_shadow);
    
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    
    // Pass vertices
    glEnableVertexAttribArray(gvPositionHandle);
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid*) 0);
    checkGlError("gvPositionHandle");
    
    // Pass color texture of g buffer
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pipeline->colorTexture);
    GLuint colorTextureUniform = glGetUniformLocation(colorShader, "u_ColorTexture");
    glUniform1i(colorTextureUniform, 0);
    checkGlError("albTextureUniform");
   
    // Pass geometry texture of g buffer
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pipeline->geometryTexture);
    GLuint geometryTextureUniform = glGetUniformLocation(colorShader, "u_GeometryTexture");
    glUniform1i(geometryTextureUniform, 1);
    checkGlError("albTextureUniform");
    
    // Pass shadow texture
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, pipeline->shadowTexture);
    GLuint shadowTextureUniform = glGetUniformLocation(colorShader, "u_ShadowTexture");
    glUniform1i(shadowTextureUniform, 2);
    checkGlError("shadowTextureUniform");
    
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
    checkGlError("glDrawArrays");
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
}
