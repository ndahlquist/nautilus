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

Character::Character(const char *objFilename, const char *vertexShaderFilename, const char *fragmentShaderFilename)
                                                  : RenderObject(objFilename, vertexShaderFilename, fragmentShaderFilename)  {
    position = Vector3f(0, 0, 0);
    targetPosition = Vector3f(0, 0, 0);
    velocity = Vector3f(0, 0, 0);
    timer.reset();
    rot[0] = 0;
    rot[1] = 0;
    MaxVelocity = 800.0f;
    MaxAcceleration = 660.0f;
    Drag = 550.0f;
}

static inline float clamp(float x, float a, float b) {
    return x < a ? a : (x > b ? b : x);
}

void Character::Update() {

    float timeElapsed = timer.getSeconds();
    timer.reset();
    
    // Accelerate towards target
    if(targetPosition != position) {
        Vector3f targetVector = targetPosition - position;
        
        if(velocity.norm() != 0.0f)
            velocity += Drag * -velocity.normalized() * timeElapsed;
        
        if(targetVector.norm() > 20.0f) {
            float accel = clamp(MaxAcceleration / ACCELERATION_MULTIPLIER * targetVector.norm(), 0.0, MaxAcceleration);
            velocity += accel * targetVector.normalized() * timeElapsed;
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
        velocity(i) = clamp(velocity(i), -MaxVelocity, MaxVelocity);
    
    // Calculate rotation
    if(velocity.norm() > 20.0f) {
        rot[0] = atan2(velocity(0), velocity(2)) + PI / 2;
        rot[1] = acos(velocity.normalized()(1)) - PI / 2;
    } else {
        rot[1] *= .99;
    }
    
    position += velocity * timeElapsed;
    
}


