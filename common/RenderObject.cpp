//  RenderObject.cpp
//  nativeGraphics

#include "RenderObject.h"
#include "RenderPipeline.h"
#include "glsl_helper.h"
#include "obj_parser.h"
#include "transform.h"
#include "common.h"
#include "log.h"
#include "Timer.h"

Timer timer;

RenderObject::RenderObject(const char *vertexShaderFilename, const char *fragmentShaderFilename, bool writegeometry) {
    timer.reset();
    BasicInit(vertexShaderFilename, fragmentShaderFilename, writegeometry);
}

RenderObject::RenderObject(const char *objFilename, const char *vertexShaderFilename, const char *fragmentShaderFilename, bool writegeometry) {
    BasicInit(vertexShaderFilename, fragmentShaderFilename, writegeometry);

    // Parse obj file into an interleaved float buffer
    GLfloat * interleavedBuffer = getInterleavedBuffer((char *)loadResource(objFilename), numVertices, true, true);
    glGenBuffers(1, &gVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, numVertices * (3+3+2) * sizeof(float), interleavedBuffer, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    checkGlError("VertexBuffer Generation");
    free(interleavedBuffer);
}

void RenderObject::BasicInit(const char *vertexShaderFilename, const char *fragmentShaderFilename, bool writegeometry) {
    numVertices=0;
    
    // Compile and link shader program
    const char * vertexShader = "standard_v.glsl";
    if(vertexShaderFilename)
        vertexShader = vertexShaderFilename;
    colorShader = createShaderProgram((char *)loadResource(vertexShader), (char *)loadResource(fragmentShaderFilename));
    SetShader(colorShader);
    
    if(vertexShaderFilename && writegeometry)
        geometryShader = createShaderProgram((char *)loadResource(vertexShaderFilename), (char *)loadResource("geometry_f.glsl"));
    else
        geometryShader = -1;
    
    texture = -1;
    normalTexture = -1;
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
    normalMapUniform = glGetUniformLocation(shaderProgram, "u_NormalMap");
    timeUniform = glGetUniformLocation(shaderProgram, "u_Time");
    checkGlError("glGetAttribLocation");
}

void RenderObject::AddTexture(const char *textureFilename, bool normalmap) {
    // Load textures
    int width, height;
    GLubyte *imageData = (GLubyte *)loadResource(textureFilename, &width, &height);
    
    GLuint newTex = -1;
    glGenTextures(1, &newTex);
    glBindTexture(GL_TEXTURE_2D, newTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    free(imageData); // TODO: Not allowed on Samsung Galaxy (not malloc'd).
    
    checkGlError("AddTexture");
    
    if(normalmap)
        normalTexture = newTex;
    else
        texture = newTex;
}

// Renders to the currently-active frame buffer.
void RenderObject::RenderPass(int instance, GLfloat *buffer, int num) {

    // Pass matrices
    GLfloat* mv_Matrix = (GLfloat*)mvMatrix();
    GLfloat* mvp_Matrix = (GLfloat*)mvpMatrix();
    glUniformMatrix4fv(gmvMatrixHandle, 1, GL_FALSE, mv_Matrix);
    glUniformMatrix4fv(gmvpMatrixHandle, 1, GL_FALSE, mvp_Matrix);
    checkGlError("glUniformMatrix4fv");
    delete[] mv_Matrix;
    delete[] mvp_Matrix;
    
    if(buffer != NULL)
        glBindBuffer(GL_ARRAY_BUFFER, 0); // Don't use vertex buffer
    else
        glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    
    // Pass vertices
    glEnableVertexAttribArray(gvPositionHandle);
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid*)(0 + buffer));
    checkGlError("gvPositionHandle");
    
    // Pass normals
    if(gvNormals != -1) {
        glEnableVertexAttribArray(gvNormals);
        glVertexAttribPointer(gvNormals, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid*)(3 + buffer));
        checkGlError("gvNormals");
    }
    
    // Pass texture coords
    if(gvTexCoords != -1) {
    	glEnableVertexAttribArray(gvTexCoords);
    	glVertexAttribPointer(gvTexCoords, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid*)(6 + buffer));
    	checkGlError("gvTexCoords");
    }
    
    // Pass texture
    if(textureUniform != -1 && texture != -1) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(textureUniform, 0);
        checkGlError("texture");
    }
    
    if(timeUniform != -1)
        glUniform1f(timeUniform, timer.getSeconds());

    // Pass normal map
    if(normalMapUniform != -1 && normalTexture != -1) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalTexture);
        glUniform1i(textureUniform, 1);
        checkGlError("normalTexture");
    }
    
    if(buffer != NULL)
        glDrawArrays(GL_TRIANGLES, 0, num);
    else
        glDrawArrays(GL_TRIANGLES, 0, numVertices);

    checkGlError("glDrawArrays");

}

void RenderObject::Render(int instance, GLfloat *buffer, int num) {

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
    glViewport(0, 0, displayWidth, displayHeight);
    
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);
    checkGlError("glClear");
    
    RenderPass(instance, buffer, num);
    
    // Render geometry (NX_MV, NY_MV, NZ_MV, Depth_MVP)
    if(geometryShader != -1)
        SetShader(geometryShader);
    else
        SetShader(pipeline->geometryShader);
    
    glBindFramebuffer(GL_FRAMEBUFFER, pipeline->frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pipeline->geometryTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pipeline->geometryDepthBuffer);
    glViewport(0, 0, pipeline->geometryTextureWidth, pipeline->geometryTextureHeight);
    
    glEnable(GL_DITHER);
    
    RenderPass(instance, buffer, num);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0); // TODO: unbind other resources
}

