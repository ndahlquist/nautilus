//  RenderObject.cpp
//  nativeGraphics

#include "RenderObject.h"
#include "RenderPipeline.h"
#include "glsl_helper.h"
#include "obj_parser.h"
#include "transform.h"
#include "common.h"
#include "log.h"

RenderObject::RenderObject(const char *objFilename, const char *vertexShaderFilename, const char *fragmentShaderFilename) {
    // Parse obj file into an interleaved float buffer
    GLfloat * interleavedBuffer = getInterleavedBuffer((char *)resourceCallback(objFilename), numVertices, true, true);
    glGenBuffers(1, &gVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, numVertices * (3+3+2) * sizeof(float), interleavedBuffer, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    checkGlError("VertexBuffer Generation");
    free(interleavedBuffer);
    
    // Compile and link shader program
    colorShader = createShaderProgram((char *)resourceCallback(vertexShaderFilename), (char *)resourceCallback(fragmentShaderFilename));
    SetShader(colorShader);
    
    texture = -1;
}

void RenderObject::SetShader(const GLuint shaderProgram) {
    glUseProgram(shaderProgram);
    checkGlError("glUseProgram");

    gmvMatrixHandle = glGetUniformLocation(shaderProgram, "u_MVMatrix");
    gmvpMatrixHandle = glGetUniformLocation(shaderProgram, "u_MVPMatrix");
    gvPositionHandle = glGetAttribLocation(shaderProgram, "a_Position");
    gvNormals = glGetAttribLocation(shaderProgram, "a_Normal");
    gvTexCoords = glGetAttribLocation(shaderProgram, "a_TexCoordinate");
    textureUniform = glGetUniformLocation(shaderProgram, "u_Texture");
    checkGlError("glGetAttribLocation");
}

void RenderObject::AddTexture(const char *textureFilename) {
    // Load textures
    GLubyte *imageData = (GLubyte *)resourceCallback(textureFilename);
    
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData); // TODO: hardcoded size

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    free(imageData);
    
    checkGlError("AddTexture");
}

void RenderObject::RenderPass() {
    // Pass matrices
    GLfloat* mv_Matrix = (GLfloat*)mvMatrix();
    GLfloat* mvp_Matrix = (GLfloat*)mvpMatrix();
    glUniformMatrix4fv(gmvMatrixHandle, 1, GL_FALSE, mv_Matrix);
    glUniformMatrix4fv(gmvpMatrixHandle, 1, GL_FALSE, mvp_Matrix);
    checkGlError("glUniformMatrix4fv");
    delete mv_Matrix;
    delete mvp_Matrix;
    
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    
    // Pass vertices
    glEnableVertexAttribArray(gvPositionHandle);
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid*) 0);
    checkGlError("gvPositionHandle");
    
    // Pass normals
    if(gvNormals != -1) {
        glEnableVertexAttribArray(gvNormals);
        glVertexAttribPointer(gvNormals, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid*) (3 * sizeof(GLfloat)));
        checkGlError("gvNormals");
    }
    
    // Pass texture coords
    if(gvTexCoords != -1) {
    	glEnableVertexAttribArray(gvTexCoords);
    	glVertexAttribPointer(gvTexCoords, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid *) (6 * sizeof(GLfloat)));
    	checkGlError("gvTexCoords");
    }
    
    // Pass texture
    if(textureUniform != -1 && texture != -1) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(textureUniform, 0);
        checkGlError("texture");
    }
    
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
    checkGlError("glDrawArrays");

}

void RenderObject::RenderFrame() {

    if(!pipeline) {
        LOGE("RenderPipeline inaccessible.");
        exit(0);
    }
    
    //////////////////////////////////
    // Render to frame buffer
    
    // Render colors (R, G, B, UNUSED / SPECULAR)
    SetShader(colorShader);
    
    glBindFramebuffer(GL_FRAMEBUFFER, pipeline->frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pipeline->colorTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pipeline->depthBuffer);
    
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);
    checkGlError("glClear");
    
    RenderPass();
    
    // Render geometry (NX_MV, NY_MV, NZ_MV, Depth_MVP)
    SetShader(pipeline->geometryShader);
    
    glBindFramebuffer(GL_FRAMEBUFFER, pipeline->frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pipeline->geometryTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pipeline->depthBuffer);
    
    glDepthMask(GL_FALSE); // We share the same depth buffer here, so don't overwrite it.
    glDepthFunc(GL_EQUAL);
    
    RenderPass();
    
    glDepthMask(GL_TRUE); // TODO
    
    glBindBuffer(GL_ARRAY_BUFFER, 0); // TODO: unbind other resources
}
