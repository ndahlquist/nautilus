//
//  RenderObject.cpp
//  nativeGraphics
//
//  Created by Ling-Ling Zhang on 5/9/13.
//  Copyright (c) 2013 Ling-Ling Zhang. All rights reserved.
//

#include "RenderObject.h"
#include "glsl_helper.h"
#include "obj_parser.h"
#include "transform.h"

RenderObject::RenderObject(char *objFile, const char *vertexShaderFile, const char *fragmentShaderFile) {

    // Parse obj file into an interleaved float buffe
    GLfloat * interleavedBuffer = getInterleavedBuffer(objFile, numVertices, true, true);
    glGenBuffers(1, &gVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, numVertices * (3+3+2) * sizeof(float), interleavedBuffer, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    checkGlError("VertexBuffer Generation");
    free(interleavedBuffer);
    
    // Compile and link shader program
    gProgram = createProgram(vertexShaderFile, fragmentShaderFile);
    
    // Get uniform and attrib locations
    gmvMatrixHandle = glGetUniformLocation(gProgram, "u_MVMatrix");
    gmvpMatrixHandle = glGetUniformLocation(gProgram, "u_MVPMatrix");
    gvPositionHandle = glGetAttribLocation(gProgram, "a_Position");
    gvNormals = glGetAttribLocation(gProgram, "a_Normal");
    gvTexCoords = glGetAttribLocation(gProgram, "a_TexCoordinate");
    checkGlError("glGetAttribLocation");
}

void RenderObject::RenderFrame()
{
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
    
    // Normals
    glEnableVertexAttribArray(gvNormals);
    glVertexAttribPointer(gvNormals, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid*) (3 * sizeof(GLfloat)));
    checkGlError("gvNormals");
    
    /*
     glActiveTexture(GL_TEXTURE0);
     glUniform1i(textureUniform, 0);
     checkGlError("texture");
     */
    // Texture
    //glEnableVertexAttribArray(gvTexCoords);
    //glVertexAttribPointer(gvTexCoords, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, raptorVertices + (sizeof(GLfloat) * 6));
    //checkGlError("gvTexCoords");
    
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
    checkGlError("glDrawArrays");
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
