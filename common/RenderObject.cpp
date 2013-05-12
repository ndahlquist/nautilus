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
#include "common.h"

RenderObject::RenderObject(const char *objFilename, const char *vertexShaderFilename, const char *fragmentShaderFilename) {
    
    // Parse obj file into an interleaved float buffe
    GLfloat * interleavedBuffer = getInterleavedBuffer((char *)resourceCallback(objFilename), numVertices, true, true);
    glGenBuffers(1, &gVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, numVertices * (3+3+2) * sizeof(float), interleavedBuffer, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    checkGlError("VertexBuffer Generation");
    free(interleavedBuffer);
    
    // Compile and link shader program
    gProgram = createProgram((char *)resourceCallback(vertexShaderFilename), (char *)resourceCallback(fragmentShaderFilename));
    
    // Get uniform and attrib locations
    gmvMatrixHandle = glGetUniformLocation(gProgram, "u_MVMatrix");
    gmvpMatrixHandle = glGetUniformLocation(gProgram, "u_MVPMatrix");
    gvPositionHandle = glGetAttribLocation(gProgram, "a_Position");
    gvNormals = glGetAttribLocation(gProgram, "a_Normal");
    gvTexCoords = glGetAttribLocation(gProgram, "a_TexCoordinate");
    textureUniform = glGetUniformLocation(gProgram, "Texture");
    checkGlError("glGetAttribLocation");
    
    texture_count = 0;
}

void RenderObject::AddTexture(const char *textureFilename)
{
    // Load textures
    GLubyte *imageData = (GLubyte *)resourceCallback("raptor.jpg");
     
    GLuint texName;
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    free(imageData);
    texture = texName;
    texture_count++;
    
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
    
    if (texture_count > 0) {
        // Texture
        glEnableVertexAttribArray(gvTexCoords);
        glVertexAttribPointer(gvTexCoords, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, (const GLvoid *) (6 * sizeof(GLfloat)));
        checkGlError("gvTexCoords");
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(textureUniform, 0);
        checkGlError("texture");
    }
    
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
    checkGlError("glDrawArrays");
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
