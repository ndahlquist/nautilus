//  common.cpp
//  nativeGraphics

#include "common.h"

#include <string>

#ifdef ANDROID_NDK
    #include "importgl.h"
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
    #include <jni.h>
#elif __APPLE__
    #include <stdlib.h>
    #include <OpenGLES/ES2/gl.h>
#else // linux
    #include <GL/glew.h>
    #include <stdio.h>
#endif

#include "Eigen/Core"
#include "Eigen/Eigenvalues"

#include "transform.h"
#include "obj_parser.h"
#include "glsl_helper.h"

#define  LOG_TAG    "libnativegraphics"
#include "log.h"

using namespace std;

GLuint gProgram;
GLuint gvPositionHandle;
GLuint gmvMatrixHandle;
GLuint gmvpMatrixHandle;
GLuint textureUniform;
GLuint gvTexCoords;

// Callback function to load resources.
void*(*resourceCallback)(const char *) = NULL;

void SetResourceCallback(void*(*cb)(const char *)) {
    resourceCallback = cb;
}

GLfloat * raptorVertices = NULL;
int raptorVerticesSize = 0;

//
// Initialize the application, loading all of the settings that
// we will be accessing later in our fragment shaders.
//
void Setup(int w, int h) {
    
    if(!resourceCallback) {
        LOGE("Resource callback not set.");
        exit(-1);
    }

    // Log device-specific GL info
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    // Parse obj file into an interleaved float buffer
    char * objFile = (char *)resourceCallback("raptor.obj");
    raptorVertices = getInterleavedBuffer(objFile, raptorVerticesSize, false, true);
    free(objFile);
        
    // Compile and link shader program
    gProgram = createProgram((char*)resourceCallback("standard_v.glsl"), NULL);
    
    // Get uniform and attrib locations
    gmvMatrixHandle = glGetUniformLocation(gProgram, "u_MVMatrix");
    gmvpMatrixHandle = glGetUniformLocation(gProgram, "u_MVPMatrix");
    gvPositionHandle = glGetAttribLocation(gProgram, "a_Position");
    checkGlError("glGetAttribLocation");
    
    // Load textures
    /*gvTexCoords = glGetAttribLocation(gProgram, "a_TexCoordinate");
    
    void *imageData = resourceCallback("raptor.jpg");
    textureUniform = glGetUniformLocation(gProgram, "Texture");
    
    GLuint texName;
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    free(imageData);*/
    
    glViewport(0, 0, w, h);
    checkGlError("glViewport");
}

const GLfloat gTriangleVertices[] = { 0.0f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f };
float cameraPos[4] = {0,0,0.9,1};
float pan[3] = {0,0,0}, up[3] = {0,1,0};
void RenderFrame() {
    
    if(!raptorVertices) {
        LOGE("raptorVertices undeclared");
        return;
    }
    static float delta = 0.01f;
    static float grey;
    grey += delta;
    if(grey > 1.0f || grey < 0.0f) {
        delta *= -1;
        grey += delta;
    }
    glClearColor(grey, .8f * grey, grey, 1.0f);
    checkGlError("glClearColor");
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");

    pLoadIdentity();
    //viewport(0, 0, 100, 200);
    //perspective(20, 0.5, 0.001, 1000);
    frustum(-0.6, 0.6, -1, 1, 0.2, 10);
    
    mvLoadIdentity();
    lookAt(cameraPos[0]+pan[0], cameraPos[1]+pan[1], cameraPos[2]+pan[2], pan[0], pan[1], pan[2], up[0], up[1], up[2]);
    
    rotatef(35,0,1,0);
    scalef(1.1, 1.1, 1.1);
    translatef(0.1f, -.2f, 0.f);
    
    glUseProgram(gProgram);
    checkGlError("glUseProgram");
    
    //glActiveTexture(GL_TEXTURE0);
    //glUniform1i(textureUniform, 0);
    //checkGlError("texture");
    
    GLfloat* mv_Matrix = (GLfloat*)mvMatrix();
    GLfloat* mvp_Matrix = (GLfloat*)mvpMatrix();
    
    glUniformMatrix4fv(gmvMatrixHandle, 1, GL_FALSE, mv_Matrix);
    glUniformMatrix4fv(gmvpMatrixHandle, 1, GL_FALSE, mvp_Matrix);
    checkGlError("glUniformMatrix4fv");
    delete mv_Matrix;
    delete mvp_Matrix;
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*5, raptorVertices);
    checkGlError("glVertexAttribPointer");
    glVertexAttribPointer(gvTexCoords, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*5, raptorVertices + (sizeof(GLfloat) * 3));
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(gvPositionHandle);
    checkGlError("glEnableVertexAttribArray");
    glDrawArrays(GL_TRIANGLES, 0, raptorVerticesSize);
    checkGlError("glDrawArrays");
}

#undef LOG_TAG
