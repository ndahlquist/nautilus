//  Character.h
//  nativeGraphics

#ifndef __nativeGraphics__Character__
#define __nativeGraphics__Character__

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

#include <vector>

#include "RenderObject.h"
#include "Timer.h"

#include "Eigen/Core"

using Eigen::Vector3f;

using namespace std;


struct DestructibleNode;
struct DestructibleBond;
struct DestructibleFace;
struct DestructibleCell;

struct characterInstance {
    characterInstance() {
        targetPosition = Vector3f(0, 0, 0);
        MaxVelocity = 800.0f;
        MaxAcceleration = 660.0f;
        Drag = 550.0f;
        position = Vector3f(0, 0, 0);
        velocity = Vector3f(0, 0, 0);
        rot[0] = 0;
        rot[1] = 0;
        lastUpdate.reset();
        animationTime = 0.0f;
    }
    
    Vector3f targetPosition;
    float MaxVelocity;
    float MaxAcceleration;
    float Drag;
    
    Vector3f position;
    Vector3f velocity;    
    float rot[2];
    
    Timer lastUpdate;
    float animationTime;
};

class Character : public RenderObject {
public:
    Character(const char *objFile, const char *vertexShaderFile, const char *fragmentShaderFile, bool collisions = false);
    void Update(); // Update all instances
    void Update(int instance); // Update a specific instance
    void RenderPass(int instance);
    /*void ReplaceModel(); //should only be used for sub, replaces sub with destructible model
    void DestructibleRender();
    void DestructibleRenderPass();
    */
    vector<struct characterInstance> instances;

    bool collide;
    bool destroyed;
};


#endif // __nativeGraphics__Character__
