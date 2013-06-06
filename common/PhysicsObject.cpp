//  PhysicsObject.cpp
//  nativeGraphics

#include "PhysicsObject.h"

#include "common.h"
#include "RenderPipeline.h"
#include "transform.h"
#include "log.h"

#include "Eigen/Eigenvalues"

using Eigen::Vector3f;
using Eigen::Vector4f;

#define MAX_VELOCITY 800.0
#define COEFF_RESTITUTION .85f

PhysicsObject::PhysicsObject(const char *objFilename, const char *vertexShaderFilename, const char *fragmentShaderFilename, bool collide)
                                                  : RenderObject(objFilename, vertexShaderFilename, fragmentShaderFilename)  {
    ScreenSpaceCollisions = collide;
    
    struct physicsInstance instance;
    instances.push_back(instance);
}

inline float clamp(float x, float a, float b) {
    return x < a ? a : (x > b ? b : x);
}

void PhysicsObject::Update() {
    for(int i = 0; i < instances.size(); i++)
        Update(i);
}

void PhysicsObject::Update(int instanceNum) {

    struct physicsInstance * instance = &instances[instanceNum];

    float timeElapsed = instance->lastUpdate.getSeconds();
    instance->lastUpdate.reset();

    instance->velocity += instance->acceleration * timeElapsed;
    
    if(ScreenSpaceCollisions) {
        Vector4f MVP_POS = projection.top()*model_view.top()*Vector4f(instance->position[0], instance->position[1], instance->position[2], 1.0); // TODO
        float x = ((1.0f + MVP_POS(0) / MVP_POS(3)) / 2.0f);
        float y = ((1.0f + MVP_POS(1) / MVP_POS(3)) / 2.0f);
        
        uint8_t * geometry = pipeline->RayTracePixel(x, y, true);
        uint8_t depth = geometry[3];
        Vector4f MV_normal = Vector4f(geometry[0] / 128.0f - 1.0f, geometry[1] / 128.0f - 1.0f, geometry[2] / 128.0f - 1.0f, 0.0);
        
        Vector4f normal_hom = model_view.top().inverse() * MV_normal;
        Vector3f normal = Vector3f(normal_hom(0), -normal_hom(1), normal_hom(2)); // TODO: Why is this negative??
        normal.normalize();
        
        delete[] geometry;
        
        if(depth < 256 * MVP_POS(2) / MVP_POS(3))
            instance->velocity = COEFF_RESTITUTION * (-2 * instance->velocity.dot(normal) * normal + instance->velocity);
    }
    
    for(int i = 0; i < 3; i++)
        instance->velocity(i) = clamp(instance->velocity(i), -MAX_VELOCITY, MAX_VELOCITY);
    
    instance->position += instance->velocity * timeElapsed;
    
}

