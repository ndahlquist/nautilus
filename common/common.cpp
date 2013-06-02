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
    
    cave = new RenderObject("box.obj", "standard_v.glsl", "solid_color_f.glsl");
    // pointLight = new RenderLight("icosphere.obj", "dr_standard_v.glsl", "dr_pointlight_f.glsl");
    spotLight = new RenderLight("icosphere.obj", "dr_standard_v.glsl", "dr_shadow_f.glsl");
    // globalLight = new RenderLight("square.obj", "dr_standard_v.glsl", "dr_normals_f.glsl");
}

void setFrameBuffer(int handle) {
    defaultFrameBuffer = handle;
}

float cameraPos[3] = {0, 10, 20};
float cameraPan[3] = {0,0,0};
float up[3] = {0,1,0};
float rot[2] = {0,0};

float characterPos[3] = {0,0,0};

#define PAN_LERP_FACTOR .02
#define CHARACTER_LERP_FACTOR .05
#define TOUCH_DISP_FACTOR 1.0f

bool touchDown = false;
float lastTouch[2] = {0,0};

void RenderFrame() {

    // Setup pipeline and perspective matrices
    glViewport(0, 0, displayWidth, displayHeight);
    
    pipeline->ClearBuffers();

    // Render shadow (in light's POV)
    pLoadIdentity();
    perspective(70, (float) displayWidth / (float) displayHeight, 0.1, 40);
    
    mvLoadIdentity();
    lookAt(characterPos[0] - 8, characterPos[1] + 8, characterPos[2], 0, 0, 0, 0, 1, 0);

    pipeline->saveShadowMatrices();
    
    cave->RenderShadow();

    // Render other stuff (switching to camera's POV)
    pLoadIdentity();
    perspective(70, (float) displayWidth / (float) displayHeight, 0.1, 40);
    
    mvLoadIdentity();
    lookAt(-5, 10, 20, 0, 0, 0, 0, 1, 0);
    //////////////////////////////////
    // Render to g buffer.
    mvPushMatrix();
    cave->RenderFrame();
    mvPopMatrix();

    ////////////////////////////////////////////////////
    // Using g buffer, render lights
    mvPushMatrix();
    translatef(characterPos[0] - 8.0, characterPos[1] + 8.0, characterPos[2]);
    scalef(18.0f);
    spotLight->brightness[0] = 200.0;
    spotLight->RenderFrame();
    mvPopMatrix();
    
    // pLoadIdentity();
    // mvLoadIdentity();
    // globalLight->RenderFrame();
    
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

