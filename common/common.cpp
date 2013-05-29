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
RenderLight *pointLight = NULL;
RenderLight *spotLight = NULL;
RenderLight *globalLight = NULL;

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
    
    pointLight = new RenderLight("icosphere.obj", "dr_standard_v.glsl", "dr_pointlight_f.glsl");
    spotLight = new RenderLight("cone.obj", "dr_standard_v.glsl", "dr_spotlight_f.glsl");
    globalLight = new RenderLight("square.obj", "dr_standard_v.glsl", "dr_normals_f.glsl");
}

void setFrameBuffer(int handle) {
    defaultFrameBuffer = handle;
}

float cameraPos[3] = {0,180,100};
float cameraPan[3] = {0,0,0};
float up[3] = {0,1,0};
float rot[2] = {0,0};

float touchTarget[3] = {0,0,0};
float characterPos[3] = {0,0,0};

#define PAN_LERP_FACTOR .02
#define CHARACTER_LERP_FACTOR .05
#define TOUCH_DISP_FACTOR 100.0f

bool touchDown = false;
float lastTouch[2] = {0,0};

void RenderFrame() {

    // Process user input
    if(touchDown) {
        touchTarget[0] = cameraPan[0] + TOUCH_DISP_FACTOR * (2.0 * lastTouch[0] - 1.0);
        touchTarget[1] = -80.0;
        touchTarget[2] = cameraPan[2] + TOUCH_DISP_FACTOR * (2.0 * lastTouch[1] - 1.0);
    }
    
    for(int i = 0; i < 3; i++) {
        characterPos[i] = (1.0 - CHARACTER_LERP_FACTOR) * characterPos[i] + CHARACTER_LERP_FACTOR * touchTarget[i];
        cameraPan[i] = (1.0 - PAN_LERP_FACTOR) * cameraPan[i] + PAN_LERP_FACTOR * characterPos[i];
    }
    
    if(abs(characterPos[2] - touchTarget[2]) > .01)
        rot[1] = atan2((characterPos[0] - touchTarget[0]), (characterPos[2] - touchTarget[2])) - 3.14 / 2;

    // Setup pipeline and perspective matrices
    glViewport(0, 0, displayWidth, displayHeight);
    
    pipeline->ClearBuffers();

    pLoadIdentity();
    perspective(40, (float) displayWidth / (float) displayHeight, 30, 420);
    
    mvLoadIdentity();
    lookAt(cameraPos[0]+cameraPan[0], cameraPos[1]+cameraPan[1], cameraPos[2]+cameraPan[2], cameraPan[0], cameraPan[1], cameraPan[2], up[0], up[1], up[2]);

    //////////////////////////////////
    // Render to g buffer.
    
    cave->RenderFrame();
    
    mvPushMatrix();
    translatef(characterPos[0], characterPos[1], characterPos[2]);
    rotate(0.0,rot[1],0);
    scalef(.3);
    character->RenderFrame();
    mvPopMatrix();

    ////////////////////////////////////////////////////
    // Using g buffer, render lights
    
    mvPushMatrix();
    translatef(characterPos[0], characterPos[1] + 30.0, characterPos[2]);
    scalef(60.0f);
    pointLight->brightness[0] = 600.0;
    pointLight->RenderFrame();
    mvPopMatrix();
    
    mvPushMatrix();
    translatef(characterPos[0], characterPos[1] + 30.0, characterPos[2]);
    scalef(60.0f);
    rotate(0.0,rot[1],0);
    rotate(0.0,0,-90);
    spotLight->brightness[0] = 2000.0;
    spotLight->RenderFrame();
    mvPopMatrix();
    
    pLoadIdentity();
    mvLoadIdentity();
    globalLight->RenderFrame();
    
    frameNum++;
}

void PointerDown(float x, float y, int pointerIndex) {
    lastTouch[0] = x;
    lastTouch[1] = y;
    touchDown = true;
}

void PointerMove(float x, float y, int pointerIndex) {
    lastTouch[0] = x;
    lastTouch[1] = y;
    touchDown = true;
}

void PointerUp(float x, float y, int pointerIndex) {
    touchDown = false;
}

