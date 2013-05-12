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
#include "RenderObject.h"

#define  LOG_TAG    "libnativegraphics"
#include "log.h"

using namespace std;

GLuint depthRenderBuffer;

int width = 0;
int height = 0;

// Callback function to load resources.
void*(*resourceCallback)(const char *) = NULL;

void SetResourceCallback(void*(*cb)(const char *)) {
    resourceCallback = cb;
}

RenderObject *cave;
RenderObject *character;

// Creates a render object given the filenames
RenderObject *createRenderObject(const char *objFilename, const char *vertexShaderFilename, const char *fragmentShaderFilename)
{
    char *objFile = (char *)resourceCallback(objFilename);
    char *vertexShaderFile = (char *)resourceCallback(vertexShaderFilename);
    char *fragmentShaderFile = (char *)resourceCallback(fragmentShaderFilename);
    
    RenderObject *retObj = new RenderObject(objFile, vertexShaderFile, fragmentShaderFile);
    
    free(objFile);
    free(vertexShaderFile);
    free(fragmentShaderFile);
    
    return retObj;
}

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
    
    cave = createRenderObject("hex.obj", "standard_v.glsl", "diffuse_f.glsl");    
    character = createRenderObject("raptor.obj", "standard_v.glsl", "diffuse_f.glsl");
    
    // Load textures
    /*
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
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0., 0., 0., .1f);
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

    cave->RenderFrame();
    character->RenderFrame();

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
