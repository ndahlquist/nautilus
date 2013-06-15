//  PhysicsObject.h
//  nativeGraphics

#ifndef __nativeGraphics__PhysicsObject__
#define __nativeGraphics__PhysicsObject__

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

#include "RenderObject.h"
#include "Timer.h"

#include <vector>

#include "Eigen/Core"

using Eigen::Vector3f;

using namespace std;

struct physicsInstance {
    physicsInstance() {
        position = Vector3f(0, 0, 0);
        velocity = Vector3f(0, 0, 0);
        acceleration = Vector3f(0, -500.0, 0);
        timer.reset();
        lastUpdate.reset();
    }

    Vector3f position;
    Vector3f velocity;
    Vector3f acceleration;
    Timer timer;
    Timer lastUpdate;
};

class PhysicsObject : public RenderObject {
public:
    PhysicsObject(const char *objFile, const char *vertexShaderFile, const char *fragmentShaderFile, bool collide = true);
    void Update(); // Update all instances
    void Update(int instance); // Update a specific instance
    
    vector<struct physicsInstance> instances;
    
private:
    bool ScreenSpaceCollisions;
};


#endif // __nativeGraphics__PhysicsObject__
