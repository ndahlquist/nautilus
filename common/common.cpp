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
GLuint gvNormals;

GLuint gVertexBuffer; 

GLuint depthRenderBuffer;

int width = 0;
int height = 0;

// Callback function to load resources.
void*(*resourceCallback)(const char *) = NULL;

void SetResourceCallback(void*(*cb)(const char *)) {
    resourceCallback = cb;
}

int numVertices = 0;

// Initialize the application, loading all of the settings that
// we will be accessing later in our fragment shaders.
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
    
    glGenRenderbuffers(1, &depthRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, w, h);

    // Parse obj file into an interleaved float buffer
    char * objFile = (char *)resourceCallback("raptor.obj");
    GLfloat * interleavedBuffer = getInterleavedBuffer(objFile, numVertices, true, true);
    free(objFile);
    glGenBuffers(1, &gVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, numVertices * (3+3+2) * sizeof(float), interleavedBuffer, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    checkGlError("VertexBuffer Generation");
    free(interleavedBuffer);
        
    // Compile and link shader program
    gProgram = createProgram((char*)resourceCallback("standard_v.glsl"), (char*)resourceCallback("diffuse_f.glsl"));
    
    // Get uniform and attrib locations
    gmvMatrixHandle = glGetUniformLocation(gProgram, "u_MVMatrix");
    gmvpMatrixHandle = glGetUniformLocation(gProgram, "u_MVPMatrix");
    gvPositionHandle = glGetAttribLocation(gProgram, "a_Position");
    gvNormals = glGetAttribLocation(gProgram, "a_Normal");
    gvTexCoords = glGetAttribLocation(gProgram, "a_TexCoordinate");
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
    
    width = w;
    height = h;
    glViewport(0, 0, w, h);
    checkGlError("glViewport");
}

float cameraPos[4] = {0,0,0.9,1};
float pan[3] = {0,0,0}, up[3] = {0,1,0};
float rot[2] = {0,0};

void RenderFrame() {
    
    if(numVertices == 0) {
        LOGE("Setup not yet called.");
        return;
    }
    
    static float delta = 0.01f;
    static float grey;
    grey += delta;
    if(grey > 1.0f || grey < 0.0f) {
        delta *= -1;
        grey += delta;
    }
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(grey, .8f * grey, grey, 1.0f);
    checkGlError("glClearColor");
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");

    pLoadIdentity();
    perspective(20, (float) width / (float) height, 10, 1000);
    
    mvLoadIdentity();
    lookAt(cameraPos[0]+pan[0], cameraPos[1]+pan[1], cameraPos[2]+pan[2], pan[0], pan[1], pan[2], up[0], up[1], up[2]);
    
    scalef(.05, .05, .05);
    translatef(0.0f, 0.0f, -600.0f);
    rotate(rot[1],rot[0],0);
    translatef(0.0f, -40.0f, 0.0f);
    
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
    
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    
    glEnableVertexAttribArray(gvNormals);
    glVertexAttribPointer(gvNormals, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid*) (3 * sizeof(GLfloat)));
    checkGlError("gvNormals");
    
    glEnableVertexAttribArray(gvPositionHandle);
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid*) 0);
    checkGlError("gvPositionHandle");
    
    //glEnableVertexAttribArray(gvTexCoords);
    //glVertexAttribPointer(gvTexCoords, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*8, raptorVertices + (sizeof(GLfloat) * 6));
    //checkGlError("gvTexCoords");
    
    glDrawArrays(GL_TRIANGLES, 0, numVertices);
    checkGlError("glDrawArrays");
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

float lastPointer[2] = {0,0};

void PointerDown(float x, float y, int pointerIndex) {
    lastPointer[0] = x;
    lastPointer[1] = y;
}

void PointerMove(float x, float y, int pointerIndex) {
     float deltaX = x - lastPointer[0];
	 float deltaY = y - lastPointer[1];
	 
	 rot[0] +=  8.0 * deltaX;
	 rot[1] += -1.0 * deltaY;
	 
	 lastPointer[0] = x;
     lastPointer[1] = y;
}

void PointerUp(float x, float y, int pointerIndex) {
    lastPointer[0] = x;
    lastPointer[1] = y;
}

#undef LOG_TAG
