//  Character.cpp
//  nativeGraphics

#include "Character.h"

#include "common.h"
#include "RenderPipeline.h"
#include "transform.h"
#include "log.h"

#include "Eigen/Eigenvalues"
#include "Vector3.h"
#include "Point3.h"

using Eigen::Vector3f;
using Eigen::Vector4f;

#define _USE_MATH_DEFINES // M_PI

//#define COEFF_RESTITUTION .85f

#define ACCELERATION_MULTIPLIER 40.0f

Character::Character(const char *objFilename, const char *vertexShaderFilename, const char *fragmentShaderFilename, bool collide)
                                                  : RenderObject(objFilename, vertexShaderFilename, fragmentShaderFilename, true) {
    this->collide = collide;
}

static inline float clamp(float x, float a, float b) {
    return x < a ? a : (x > b ? b : x);
}

void Character::Update() {
    for(int i = 0; i < instances.size(); i++)
        Update(i);
}

void Character::Update(int instanceNum) {
    if(instanceNum >= instances.size()) {
        LOGE("Character::Update: Out of bounds exception.");
        return;
    }

    struct characterInstance * instance = &instances[instanceNum];

    float timeElapsed = instance->lastUpdate.getSeconds();
    instance->lastUpdate.reset();
    
    // Accelerate towards target
    if(instance->targetPosition != instance->position) {
        Vector3f targetVector = instance->targetPosition - instance->position;
        
        if(instance->velocity.norm() != 0.0f)
            instance->velocity += instance->Drag * -instance->velocity.normalized() * timeElapsed;
        
        if(targetVector.norm() > 20.0f) {
            float accel = clamp(instance->MaxAcceleration / ACCELERATION_MULTIPLIER * targetVector.norm(), 0.0, instance->MaxAcceleration);
            instance->velocity += accel * targetVector.normalized() * timeElapsed;
        }
    }
    
    // Clamp velocity   
    for(int i = 0; i < 3; i++)
        instance->velocity(i) = clamp(instance->velocity(i), -instance->MaxVelocity, instance->MaxVelocity);
    
    // Calculate rotation
    if(instance->velocity.norm() > 20.0f) {
        instance->rot[0] = atan2(instance->velocity(0), instance->velocity(2)) + M_PI / 2;
        instance->rot[1] = acos(instance->velocity.normalized()(1)) - M_PI / 2;
    } else {
        instance->rot[1] *= .99;
    }
    
    instance->position += instance->velocity * timeElapsed;
    
    instance->animationTime += (4.0f * instance->velocity.norm() / instance->MaxVelocity + 1.0f) * timeElapsed;
    
}

void Character::RenderPass(int instance) {
    // Pass animation time
    if(timeUniform != -1) {
        glUniform1f(timeUniform, instances[instance].animationTime);
        checkGlError("glUniform1i: time");
    }
    RenderObject::RenderPass(instance, 0, -1);
}

