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
    
    glUniformMatrix4fv(gmvMatrixHandle, 1, GL_FALSE, mv_Matrix);
    glUniformMatrix4fv(gmvpMatrixHandle, 1, GL_FALSE, mvp_Matrix);
    checkGlError("glUniformMatrix4fv");
    delete mv_Matrix;
    delete mvp_Matrix;
    
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    
    // Vertices
    glEnableVertexAttribArray(gvPositionHandle);
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid*) 0);
    checkGlError("gvPositionHandle");
    
    //Textures
    /*if(textures.size() > 0) {
    	glEnableVertexAttribArray(gvTexCoords);
    	glVertexAttribPointer(gvTexCoords, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid *) (6 * sizeof(GLfloat)));
    	checkGlError("gvTexCoords");

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        glUniform1i(textureUniform, 0);
        checkGlError("texture");
    }*/
    
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
    checkGlError("glDrawArrays");
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
