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
#include "RenderLight.h"
#include "glsl_helper.h"
#include "log.h"

using namespace std;

int displayWidth = 0;
int displayHeight = 0;

GLuint defaultFrameBuffer = 0;

RenderPipeline *pipeline = NULL;

RenderObject *cave = NULL;
RenderObject *character = NULL;
RenderLight *light = NULL;

float cameraPos[4] = {0,0,0.9,1};
float pan[3] = {0,0,0}, up[3] = {0,1,0};
float rot[2] = {0,0};

unsigned int frameNum = 0;

// Callback function to load resources.
void*(*resourceCallback)(const char *) = NULL;

void SetResourceCallback(void*(*cb)(const char *)) {
    resourceCallback = cb;
}

// Initialize the application, loading all of the settings that
// we will be accessing later in our fragment shaders.
void Setup(int w, int h) {
    
    if(!resourceCallback) {
        LOGE("Resource callback not set.");
        exit(-1);
    }
    
    displayWidth = w;
    displayHeight = h;
    
    pipeline = new RenderPipeline();
    
    cave = new RenderObject("cave1.obj", "standard_v.glsl", "solid_color_f.glsl");
    character = new RenderObject("raptor.obj", "standard_v.glsl", "albedo_f.glsl");
    character->AddTexture("raptor_albedo.jpg");
    
    light = new RenderLight("square.obj", "dr_standard_v.glsl", "dr_pointlight_f.glsl");
}

void setFrameBuffer(int handle) {
    defaultFrameBuffer = handle;
}

float lightPos[3] = { 0, 200, 100 };

void RenderFrame() {

    glViewport(0, 0, displayWidth, displayHeight);
    
    pipeline->ClearBuffers();

    pLoadIdentity();
    perspective(30, (float) displayWidth / (float) displayHeight, 120, 600); // TODO
    
    mvLoadIdentity();
    lookAt(lightPos[0], lightPos[1], lightPos[2], 0, 100, 0, 0, 1, 0);

///////////////////////////////////////////

    /*pLoadIdentity();
    perspective(20, (float) displayWidth / (float) displayHeight, 80, 180);
    
    mvLoadIdentity();
    lookAt(lightPos[0], lightPos[1], lightPos[2], 0, 0, 0, 0, 1, 0);*/

    mvPushMatrix();
    scalef(.4);
    translatef(0.0f, 0.0f, -120.0f / .4f);
    //rotate(rot[1],rot[0],0);
    //translatef(0.0f, 5.0f / .4f, 0.0f);
    cave->RenderFrame();
    mvPopMatrix();
    
    mvPushMatrix();
    scalef(.2);
    translatef(0.0f, 0.0f, -120.0f / .2f);
    //rotate(rot[1],rot[0],0);
    //translatef(68.0f, -40.0f, -20.0f); 
    character->RenderFrame();
    mvPopMatrix();

    //////////////////////////////////
    // Render to g buffer.
    
    /*mvPushMatrix();
    scalef(.4);
    translatef(0.0f, 0.0f, -120.0f / .4f);
    rotate(rot[1],rot[0],0);
    translatef(0.0f, 5.0f / .4f, 0.0f);
    cave->RenderFrame();
    mvPopMatrix();
    
    mvPushMatrix();
    scalef(.2);
    translatef(0.0f, 0.0f, -120.0f / .2f);
    rotate(rot[1],rot[0],0);
    translatef(68.0f, -40.0f, -20.0f); 
    character->RenderFrame();
    mvPopMatrix();*/

    ////////////////////////////////////////////////////
    // Using g buffer, render lights
    
    
    pLoadIdentity();
    mvLoadIdentity();
    light->RenderFrame();
    
    /*float lightScale = 15.0f;
    
    for(int i = 0; i < 3; i++)
        light->brightness[i] = 40;
    
    mvPushMatrix();
    scalef(lightScale);
    translatef(0.0f, 0.0f, -120.0f / lightScale);
    rotate(rot[1],rot[0],0);
    translatef(3.0 * cos(frameNum / 50.0f) / lightScale, -10.0f / lightScale, 30.0 * sin(frameNum / 100.0f) / lightScale);
    light->RenderFrame();
    mvPopMatrix();
    
    mvPushMatrix();
    scalef(lightScale);
    translatef(0.0f, 0.0f, -120.0f / lightScale);
    rotate(rot[1],rot[0],0);
    translatef(16.0 * sin(frameNum / 20.0f) / lightScale, -10.0f / lightScale, 16.0 * cos(frameNum / 20.0f) / lightScale);
    light->RenderFrame();
    mvPopMatrix();
    
    lightScale = 20.0f;
    
    for(int i = 0; i < 3; i++)
        light->brightness[i] = 120;
    
    mvPushMatrix();
    scalef(lightScale);
    translatef(0.0f, 0.0f, -120.0f / lightScale);
    rotate(rot[1],rot[0],0);
    translatef(-6.0f / lightScale, 10.0f / lightScale, 0.0f);
    light->RenderFrame();
    mvPopMatrix();
    
    frameNum++;*/
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

