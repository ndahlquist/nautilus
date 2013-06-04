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

#include "Eigen/Core"

using Eigen::Vector3f;

using namespace std;

class PhysicsObject : public RenderObject {
public:
    PhysicsObject(const char *objFile, const char *vertexShaderFile, const char *fragmentShaderFile);
    void Update();
    
    Vector3f position;
    Vector3f velocity;
    Vector3f acceleration;
    
    bool ScreenSpaceCollisions;
};


#endif // __nativeGraphics__PhysicsObject__
