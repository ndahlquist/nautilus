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
#include "levels/level0.h"
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

void Setup(int w, int h) {
    if(!resourceCallback) {
        LOGE("Resource callback not set.");
        exit(-1);
    }
    displayWidth = w;
    displayHeight = h;
    pipeline = new RenderPipeline();

    // level = new level0();
    level = new level1();
}

void setFrameBuffer(int handle) {
    defaultFrameBuffer = handle;
}

void RenderFrame() {
    pipeline->ClearBuffers();
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

