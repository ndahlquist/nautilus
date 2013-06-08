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
RenderDestructible *destructible = NULL;
PhysicsObject *bomb = NULL;
#define BOMB_TIMER_LENGTH 2.0f
#define BOMB_EXPLOSION_LENGTH .3f

RenderLight *smallLight = NULL;
RenderLight *bigLight = NULL;
RenderLight *explosiveLight = NULL;
RenderLight *spotLight = NULL;

#define PI 3.1415f

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
    
    cave = new RenderObject("cave2.obj", NULL, "albedo_f.glsl");
    cave->AddTexture("cave_albedo.jpg", false);
    
    character = new Character("submarine.obj", NULL, "albedo_f.glsl");
    character->AddTexture("submarine_albedo.jpg", false);
    struct characterInstance instance;
    character->instances.push_back(instance);
    
    jellyfish = new Character("jellyfish.obj", "jellyfish_v.glsl", "albedo_f.glsl");
    jellyfish->AddTexture("jellyfish_albedo.jpg", false);
    
    destructible = new RenderDestructible("cube.obj", NULL, "albedo_f.glsl");
    destructible->AddTexture("submarine_albedo.jpg", false);
    
    bomb = new PhysicsObject("icosphere.obj", NULL, "solid_color_f.glsl");
    
    smallLight = new RenderLight("icosphere.obj", "dr_standard_v.glsl", "dr_pointlight_sat_f.glsl");
    bigLight = new RenderLight("square.obj", "dr_square_v.glsl", "dr_pointlight_f.glsl");
    explosiveLight = new RenderLight("icosphere.obj", "dr_standard_v.glsl", "dr_explosive_pointlight_f.glsl");
    spotLight = new RenderLight("cone.obj", "dr_standard_v.glsl", "dr_spotlight_f.glsl");
}

void setFrameBuffer(int handle) {
    defaultFrameBuffer = handle;
}

float cameraPos[3] = {0,180,100};
float cameraPan[3] = {0,200,0};

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

void addJellyfish() {
    struct characterInstance instance;
    instance.position = character->instances[0].position + 600.0f * Vector3f((rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f);
    instance.MaxAcceleration = 200.0f;
    instance.Drag = 100.0f;
    instance.MaxVelocity = 100.0f;
    jellyfish->instances.push_back(instance);
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
    
    mvPushMatrix();
    destructible->Render();
    mvPopMatrix();
    
    // Process user input
    if(touchDown) {
        uint8_t * geometry = pipeline->RayTracePixel(lastTouch[0], 1.0f - lastTouch[1], true);
        if(geometry[3] != 255) {
            float depth = geometry[3] / 128.0f - 1.0f;            
            Matrix4f mvp = projection.top()*model_view.top();
            Vector4f pos = mvp.inverse() * Vector4f((lastTouch[0]) * 2.0f - 1.0f, (1.0 - lastTouch[1]) * 2.0f - 1.0f, depth, 1.0);
            character->instances[0].targetPosition = Vector3f(pos(0) / pos(3), pos(1) / pos(3), pos(2) / pos(3));
        }
        delete[] geometry;
    }
    
    for(int i = 0; i < 3; i++)
        cameraPan[i] = (1.0 - PAN_LERP_FACTOR) * cameraPan[i] + PAN_LERP_FACTOR * character->instances[0].position[i];
        
    if(shootBomb) {
        struct physicsInstance newBomb;
        newBomb.position = character->instances[0].position;
        newBomb.velocity = 200.0f * Eigen::Vector3f(-cos(character->instances[0].rot[0]), 1.0f, sin(character->instances[0].rot[0]));
        bomb->instances.push_back(newBomb);
        shootBomb = false;
    }
    
    while(jellyfish->instances.size() < 15)
        addJellyfish();
    
    // Run physics.
    bomb->Update();
    character->Update();
    for(int i = 0; i < jellyfish->instances.size(); i++) {
        jellyfish->instances[i].targetPosition = character->instances[0].position;
        // Randomize movement
        float dist = (character->instances[0].position - jellyfish->instances[i].position).norm();
        jellyfish->instances[i].targetPosition += 1.1f * dist * Vector3f((rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f, (rand() % 200 - 100) / 100.0f);
    }
    jellyfish->Update();
    
    mvPushMatrix();
    translate(character->instances[0].position);
    rotate(0.0, character->instances[0].rot[0], character->instances[0].rot[1]);
    scalef(.15f);
    character->Render();
    mvPopMatrix();
    
    for(int i = 0; i < jellyfish->instances.size(); i++) {
        mvPushMatrix();
        translate(jellyfish->instances[i].position);
        rotate(0.0, jellyfish->instances[i].rot[0], jellyfish->instances[i].rot[1]);
        rotate(0.0, 0.0, PI / 2);
        scalef(1.0f);
        jellyfish->Render(i);
        mvPopMatrix();
    }
    
    for(int i = 0; i < bomb->instances.size(); i++) {
        if(bomb->instances[i].timer.getSeconds() <= BOMB_TIMER_LENGTH) {
            mvPushMatrix();
            translate(bomb->instances[i].position);
            scalef(10);
            bomb->Render(i);
            mvPopMatrix();
        }
    }

    ////////////////////////////////////////////////////
    // Using g buffer, render lights
    
    mvPushMatrix();
    translate(character->instances[0].position);
    bigLight->color[0] = 1.0;
    bigLight->color[1] = 1.0;
    bigLight->color[2] = 0.8;
    bigLight->brightness = 16000.0;
    bigLight->Render();
    mvPopMatrix();
    
    for(int i = 0; i < bomb->instances.size(); i++) {
        if(bomb->instances[i].timer.getSeconds() <= BOMB_TIMER_LENGTH) {
            mvPushMatrix();
            translate(bomb->instances[i].position);
            scalef(100);
            smallLight->color[0] = 1.00f;
            smallLight->color[1] = 0.33f;
            smallLight->color[2] = 0.07f;
            smallLight->brightness = 1500 + 1500 * sin(bomb->instances[i].timer.getSeconds() * 4.0f * PI);
            smallLight->Render();
            mvPopMatrix();
        } else if(bomb->instances[i].timer.getSeconds() <= BOMB_TIMER_LENGTH + BOMB_EXPLOSION_LENGTH) {
            mvPushMatrix();
            translate(bomb->instances[i].position);
            scalef(250);
            explosiveLight->color[0] = 1.00f;
            explosiveLight->color[1] = 1.00f;
            explosiveLight->color[2] = 1.00f;
            float explosionTime = (bomb->instances[i].timer.getSeconds() - BOMB_TIMER_LENGTH) / BOMB_EXPLOSION_LENGTH;
            explosiveLight->brightness = 10000000.0f * sin(PI * sqrt(explosionTime));
            explosiveLight->Render();
            mvPopMatrix();
            for(int j = 0; j < jellyfish->instances.size(); j++) {
                if((jellyfish->instances[j].position - bomb->instances[i].position).norm() < 200) {
                    // Kill this jellyfish
                    jellyfish->instances.erase(jellyfish->instances.begin()+j);
                    j--;  
                }
            }  
        } else {
            bomb->instances.erase(bomb->instances.begin()+i);
            i--;
        }
    }
    
    mvPushMatrix();
    translate(character->instances[0].position);
    rotate(0.0, character->instances[0].rot[0], character->instances[0].rot[1]);
    rotate(0.0,0,-PI / 2);
    scalef(300.0f);
    spotLight->color[0] = 0.4f;
    spotLight->color[1] = 0.6f;
    spotLight->color[2] = 1.0f;
    spotLight->brightness = 16000.0;
    spotLight->Render();
    mvPopMatrix();
    
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

