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

#include "RenderObject.h"
#include "Timer.h"

#include "Eigen/Core"

using Eigen::Vector3f;

using namespace std;

class Character : public RenderObject {
public:
    Character(const char *objFile, const char *vertexShaderFile, const char *fragmentShaderFile);
    void Update();

    Vector3f targetPosition;
    float MaxVelocity;
    float MaxAcceleration;
    float Drag;
    
    Vector3f position;
    
    float rot[2];
    
private:
    Vector3f velocity;
    Timer timer;
};


#endif // __nativeGraphics__Character__
