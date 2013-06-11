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
#include "PhysicsObject.h"
#include "Character.h"
#include "RenderLight.h"
#include "RenderDestructible.h"
#include "Fluid.h"
#include "Timer.h"
#include "glsl_helper.h"
#include "log.h"

#include "levels/basicLevel.h"
#include "levels/level1.h"

using namespace std;
using Eigen::Matrix4f;
using Eigen::Vector4f;

int displayWidth = 0;
int displayHeight = 0;
bool touchDown = false;
float lastTouch[2] = {0,0};
float orientation[3] = {0,0,0};
GLuint defaultFrameBuffer = 0;
RenderPipeline * pipeline = NULL;

basicLevel * level = NULL;

// Callback function to load resources.
void*(*resourceCallback)(const char *, int *, int *) = NULL;

void * loadResource(const char * fileName, int * width, int * height) {
    return resourceCallback(fileName, width, height);
}

void SetResourceCallback(void*(*cb)(const char *, int *, int *)) {
    resourceCallback = cb;
}

void loadLevel() {
    if (level != NULL) {
        level->FreeLevel();
        free(level);
    }
    level = new level1("maze3x4x2.obj", Eigen::Vector3f(1000.0f, -400.0f, -000.0f));
}

void Setup(int w, int h) {
    if(!resourceCallback) {
        LOGE("Resource callback not set.");
        exit(-1);
    }
    displayWidth = w;
    displayHeight = h;
    pipeline = new RenderPipeline();

    loadLevel();
}

void setFrameBuffer(int handle) {
    defaultFrameBuffer = handle;
}

void renderDestroy() {
    for (int i = 0; i < 50; i++) {
        level->RenderFrame();
    }
}

void RenderFrame() {
    pipeline->ClearBuffers();
    
    //if(level->transitionLight > 1.0f)
        //loadLevel();
    level->RenderFrame();
    fpsMeter();
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

void UpdateOrientation(float roll, float pitch, float yaw) {
    orientation[0] = roll;
    orientation[1] = pitch;
    orientation[2] = yaw;
}

