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

    if(numVertices == 0) {
        LOGE("Setup not yet called.");
        return;
    }
    
    glUseProgram(gProgram);
    checkGlError("glUseProgram");
    
    // Matrices setup
    GLfloat* mv_Matrix = (GLfloat*)mvMatrix();
    GLfloat* mvp_Matrix = (GLfloat*)mvpMatrix();
    GLfloat* pT_Matrix = (GLfloat*)pInverseMatrix();
    
    glUniformMatrix4fv(gmvMatrixHandle, 1, GL_FALSE, mv_Matrix);
    glUniformMatrix4fv(gmvpMatrixHandle, 1, GL_FALSE, mvp_Matrix);
    GLuint gpT_MatrixHandle = glGetUniformLocation(gProgram, "u_pT_Matrix");
    glUniformMatrix4fv(gpT_MatrixHandle, 1, GL_FALSE, pT_Matrix);
    checkGlError("glUniformMatrix4fv");
    delete mv_Matrix;
    delete mvp_Matrix;
    delete pT_Matrix;
    
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    
    // Vertices
    glEnableVertexAttribArray(gvPositionHandle);
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid*) 0);
    checkGlError("gvPositionHandle");
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pipeline->colorTexture);
    GLuint colorTextureUniform = glGetUniformLocation(gProgram, "u_ColorTexture");
    glUniform1i(colorTextureUniform, 0);
    checkGlError("albTextureUniform");
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pipeline->geometryTexture);
    GLuint geometryTextureUniform = glGetUniformLocation(gProgram, "u_GeometryTexture");
    glUniform1i(geometryTextureUniform, 1);
    checkGlError("albTextureUniform");
    
    GLuint brightnessUniform = glGetUniformLocation(gProgram, "u_Brightness");
    glUniform3f(brightnessUniform, brightness[0], brightness[1], brightness[2]);
    
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
    checkGlError("glDrawArrays");
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
}
