//  Character.cpp
//  nativeGraphics

#include "Character.h"

#include "common.h"
#include "RenderPipeline.h"
#include "transform.h"
#include "log.h"

#include "Eigen/Eigenvalues"

using Eigen::Vector3f;
using Eigen::Vector4f;

#define PI 3.1415f

//#define COEFF_RESTITUTION .85f

#define ACCELERATION_MULTIPLIER 40.0f

Character::Character(const char *objFilename, const char *vertexShaderFilename, const char *fragmentShaderFilename, bool collide)
                                                  : RenderObject(objFilename, vertexShaderFilename, fragmentShaderFilename)  {
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
    
    // Check for collisions
    /*Vector4f MVP_POS = projection.top()*model_view.top()*Vector4f(position[0], position[1], position[2], 1.0);
    float x = ((1.0f + MVP_POS(0) / MVP_POS(3)) / 2.0f);
    float y = ((1.0f + MVP_POS(1) / MVP_POS(3)) / 2.0f);

    uint8_t * geometry = pipeline->RayTracePixel(x, y, true);
    uint8_t depth = geometry[3];
    Vector4f MV_normal = Vector4f(geometry[0] / 128.0f - 1.0f, geometry[1] / 128.0f - 1.0f, geometry[2] / 128.0f - 1.0f, 0.0);
        
    Vector4f normal_hom = model_view.top().inverse() * MV_normal;
    Vector3f normal = Vector3f(normal_hom(0), -normal_hom(1), normal_hom(2)); // TODO: Why is this negative??
    normal.normalize();
        
    delete[] geometry;
        
    if(depth < 256 * MVP_POS(2) / MVP_POS(3) && velocity.dot(normal) < 0)
        velocity = COEFF_RESTITUTION * (-2 * velocity.dot(normal) * normal + velocity);*/

    // Clamp velocity   
    for(int i = 0; i < 3; i++)
        instance->velocity(i) = clamp(instance->velocity(i), -instance->MaxVelocity, instance->MaxVelocity);
    
    // Calculate rotation
    if(instance->velocity.norm() > 20.0f) {
        instance->rot[0] = atan2(instance->velocity(0), instance->velocity(2)) + PI / 2;
        instance->rot[1] = acos(instance->velocity.normalized()(1)) - PI / 2;
    } else {
        instance->rot[1] *= .99;
    }
    
    instance->position += instance->velocity * timeElapsed;
    
}

void Character::RenderPass(int instance) {
    // Pass currrent time
    if(timeUniform != -1) {
        glUniform1f(timeUniform, instances[instance].timer.getSeconds());
        checkGlError("glUniform1i: time");
    }
    RenderObject::RenderPass(instance);
}


