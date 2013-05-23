//  RenderLight.cpp
//  nativeGraphics

#include "RenderLight.h"
#include "glsl_helper.h"
#include "transform.h"
#include "common.h"
#include "log.h"

RenderLight::RenderLight(const char *objFilename, const char *vertexShaderFilename, const char *fragmentShaderFilename) : RenderObject(objFilename, vertexShaderFilename, fragmentShaderFilename) {}

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
    
    //Textures
    glEnableVertexAttribArray(gvTexCoords);
    glVertexAttribPointer(gvTexCoords, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid *) (6 * sizeof(GLfloat)));
    checkGlError("gvTexCoords");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, positionTex);
    GLuint posTextureUniform = glGetUniformLocation(gProgram, "u_PosTexture");
    glUniform1i(posTextureUniform, 0);
    checkGlError("posTextureUniform");
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, albedoTex);
    GLuint albTextureUniform = glGetUniformLocation(gProgram, "u_AlbTexture");
    glUniform1i(albTextureUniform, 1);
    checkGlError("albTextureUniform");
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, normalTex);
    GLuint normTextureUniform = glGetUniformLocation(gProgram, "u_NormTexture");
    glUniform1i(normTextureUniform, 2);
    checkGlError("albTextureUniform");
    
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
    checkGlError("glDrawArrays");
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
}
