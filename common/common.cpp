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
#include "Timer.h"
#include "glsl_helper.h"
#include "log.h"

using namespace std;
using Eigen::Matrix4f;
using Eigen::Vector4f;

int displayWidth = 0;
int displayHeight = 0;

GLuint defaultFrameBuffer = 0;

RenderPipeline *pipeline = NULL;

RenderObject *cave = NULL;
Character *character = NULL;
Character *jellyfish = NULL;
PhysicsObject *bomb = NULL;

RenderLight *smallLight = NULL;
RenderLight *bigLight = NULL;
RenderLight *spotLight = NULL;

unsigned int frameNum = 0;

#define PI 3.14f

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
    
    cave = new RenderObject("cave2.obj", "standard_v.glsl", "albedo_f.glsl");
    cave->AddTexture("cave_albedo.jpg", false);
    //cave->AddTexture("cave_albedo.jpg", true); // Normal map
    character = new Character("submarine.obj", "standard_v.glsl", "albedo_f.glsl");
    character->AddTexture("submarine_albedo.jpg", false);
    jellyfish = new Character("jellyfish.obj", "jellyfish_v.glsl", "albedo_f.glsl");
    jellyfish->AddTexture("jellyfish_albedo.jpg", false);
    bomb = new PhysicsObject("icosphere.obj", "standard_v.glsl", "solid_color_f.glsl");
    
    smallLight = new RenderLight("icosphere.obj", "dr_standard_v.glsl", "dr_pointlight_sat_f.glsl");
    bigLight = new RenderLight("square.obj", "dr_square_v.glsl", "dr_pointlight_f.glsl");
    spotLight = new RenderLight("cone.obj", "dr_standard_v.glsl", "dr_spotlight_f.glsl");
}

void setFrameBuffer(int handle) {
    defaultFrameBuffer = handle;
}

float cameraPos[3] = {0,180,100};
float cameraPan[3] = {0,2000,0};

float orientation[3] = {0,0,0};

#define PAN_LERP_FACTOR .04

bool touchDown = false;
bool shootBomb = false;
float lastTouch[2] = {0,0};

// Clamps input to (-max, max) according to curve.
inline float tanClamp(float input, float max) {
    return max * atan(input / max);
}

// Rotate around the subject based on orientation
void rotatePerspective() {
    float rot0 = tanClamp( orientation[2], PI / 10.0f);
    float rot2 = tanClamp(-orientation[1], PI / 10.0f);
    
    translatef(cameraPan[0], cameraPan[1], cameraPan[2]);
    rotate(rot0, 0, rot2);
    translatef(-cameraPan[0], -cameraPan[1], -cameraPan[2]);
}

void RenderFrame() {

    pipeline->ClearBuffers();

    // Setup perspective matrices
    pLoadIdentity();
    perspective(90, (float) displayWidth / (float) displayHeight, 60, 800);
    
    mvLoadIdentity();
    lookAt(cameraPos[0]+cameraPan[0], cameraPos[1]+cameraPan[1], cameraPos[2]+cameraPan[2], cameraPan[0], cameraPan[1], cameraPan[2], 0, 1, 0);
    rotatePerspective();

    //////////////////////////////////
    // Render to g buffer.
    
    /** Any geometry that will be collision detected
        should be rendered here, before user input. **/
    mvPushMatrix();
    scalef(40);
    cave->Render();
    mvPopMatrix();
    
    // Process user input
    if(touchDown) {
        uint8_t * geometry = pipeline->RayTracePixel(lastTouch[0], 1.0f - lastTouch[1], true);
        if(geometry[3] != 255) {
            float depth = geometry[3] / 128.0f - 1.0f;            
            Matrix4f mvp = projection.top()*model_view.top();
            Vector4f pos = mvp.inverse() * Vector4f((lastTouch[0]) * 2.0f - 1.0f, (1.0 - lastTouch[1]) * 2.0f - 1.0f, depth, 1.0);
            character->targetPosition = Vector3f(pos(0) / pos(3), pos(1) / pos(3), pos(2) / pos(3));
        }
        delete[] geometry;
    }
    
    for(int i = 0; i < 3; i++)
        cameraPan[i] = (1.0 - PAN_LERP_FACTOR) * cameraPan[i] + PAN_LERP_FACTOR * character->position[i];
        
    if(shootBomb) {
        bomb->position = Eigen::Vector3f(character->position[0], character->position[1], character->position[2]);
        bomb->velocity = 200.0f * Eigen::Vector3f(-cos(character->rot[0]), 1.0f, sin(character->rot[0]));
        shootBomb = false;
    }
    
    // Run physics.
    bomb->Update();
    character->Update();
    jellyfish->targetPosition = Vector3f(character->position[0], character->position[1], character->position[2]);// character->position;
    jellyfish->Update();
    
    mvPushMatrix();
    translatef(character->position[0], character->position[1], character->position[2]);
    rotate(0.0, character->rot[0], character->rot[1]);
    scalef(.15);
    character->Render();
    mvPopMatrix();
    
    mvPushMatrix();
    translatef(jellyfish->position[0], jellyfish->position[1], jellyfish->position[2]);
    rotate(0.0, jellyfish->rot[0], jellyfish->rot[1]);
    rotate(0.0, 0.0, PI / 2);
    scalef(3.00);
    jellyfish->Render();
    mvPopMatrix();
    
    mvPushMatrix();
    translatef(bomb->position[0], bomb->position[1], bomb->position[2]);
    scalef(10);
    bomb->Render();
    mvPopMatrix();

    ////////////////////////////////////////////////////
    // Using g buffer, render lights
    
    mvPushMatrix();
    translatef(character->position[0], character->position[1], character->position[2]);
    bigLight->color[0] = 1.0;
    bigLight->color[1] = 1.0;
    bigLight->color[2] = 0.8;
    bigLight->brightness = 16000.0;
    bigLight->Render();
    mvPopMatrix();
    
    mvPushMatrix();
    translatef(bomb->position[0], bomb->position[1], bomb->position[2]);
    scalef(100);
    smallLight->color[0] = 1.00f;
    smallLight->color[1] = 0.33f;
    smallLight->color[2] = 0.07f;
    smallLight->brightness = 1500 + 1500 * sin(frameNum / 6.0f);
    smallLight->Render();
    mvPopMatrix();
    
    mvPushMatrix();
    translatef(character->position[0], character->position[1], character->position[2]);
    rotate(0.0, character->rot[0], character->rot[1]);
    rotate(0.0,0,-PI / 2);
    scalef(300.0f);
    spotLight->color[0] = 0.4f;
    spotLight->color[1] = 0.6f;
    spotLight->color[2] = 1.0f;
    spotLight->brightness = 16000.0;
    spotLight->Render();
    mvPopMatrix();
    
    frameNum++;
    
    fpsMeter();
}

void PointerDown(float x, float y, int pointerIndex) {
    lastTouch[0] = x;
    lastTouch[1] = y;
    touchDown = true;
    shootBomb = true;
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

