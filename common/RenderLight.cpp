//  RenderLight.cpp
//  nativeGraphics

#include "RenderLight.h"
#include "glsl_helper.h"
#include "transform.h"
#include "common.h"
#include "log.h"
#include "Timer.h"

Timer timer; // TODO

RenderLight::RenderLight(const char *objFilename, const char *vertexShaderFilename, const char *fragmentShaderFilename) : RenderObject(objFilename, vertexShaderFilename, fragmentShaderFilename, false) {
   color[0] = 1.0f;
   color[1] = 1.0f;
   color[2] = 1.0f;
   brightness = 1000.0f;
   timer.reset();
   
   mv_inverse_Matrix = NULL;
}

void RenderLight::PreTranslate() {
    mv_inverse_Matrix = (GLfloat*)mvInverseMatrix();
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
    GLuint u_mvMatrixHandle = glGetUniformLocation(colorShader, "u_MVMatrix");
    glUniformMatrix4fv(u_mvMatrixHandle, 1, GL_FALSE, mv_Matrix);
    delete mv_Matrix;
    
    GLfloat* mvp_Matrix = (GLfloat*)mvpMatrix();
    GLuint u_mvpMatrixHandle = glGetUniformLocation(colorShader, "u_MVPMatrix");
    glUniformMatrix4fv(u_mvpMatrixHandle, 1, GL_FALSE, mvp_Matrix);
    delete mvp_Matrix;
    
    GLfloat* p_inverse_Matrix = (GLfloat*)pInverseMatrix();
    GLuint u_p_inverseHandle = glGetUniformLocation(colorShader, "u_p_inverse");
    glUniformMatrix4fv(u_p_inverseHandle, 1, GL_FALSE, p_inverse_Matrix);
    delete p_inverse_Matrix;
    
    if(mv_inverse_Matrix) {
        GLuint u_mv_inverseHandle = glGetUniformLocation(colorShader, "u_mv_inverse");
        glUniformMatrix4fv(u_mv_inverseHandle, 1, GL_FALSE, mv_inverse_Matrix );
        //delete mv_inverse_Matrix; //TODO
    }
    checkGlError("glUniformMatrix4fv");
    
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
    
    // Pass caustic texture
    GLuint causticTextureUniform = glGetUniformLocation(colorShader, "u_CausticTexture");
    if(causticTextureUniform != -1) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, pipeline->causticTexture);
        glUniform1i(causticTextureUniform, 2);
        checkGlError("causticTextureUniform");
    }
    
    // Pass time
    GLuint timeUniform = glGetUniformLocation(colorShader, "u_Time");
    if(timeUniform != -1)
        glUniform1f(timeUniform, timer.getSeconds());

    
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
    checkGlError("glDrawArrays");
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
}
