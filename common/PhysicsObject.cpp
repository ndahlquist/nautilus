//  PhysicsObject.cpp
//  nativeGraphics

#include "PhysicsObject.h"

#include "common.h"
#include "RenderPipeline.h"
#include "transform.h"
#include "log.h"

using Eigen::Vector4f;

#define MAX_VELOCITY 1.0
#define INTERSECTION_ADD .1

PhysicsObject::PhysicsObject(const char *objFilename, const char *vertexShaderFilename, const char *fragmentShaderFilename)
                                                  : RenderObject(objFilename, vertexShaderFilename, fragmentShaderFilename)  {
    position = Vector3f(0, 0, 0);
    velocity = Vector3f(0, 0, 0);
    acceleration = Vector3f(0, -.0004, 0);
    ScreenSpaceCollisions = true;
}

inline float clamp(float x, float a, float b) {
    return x < a ? a : (x > b ? b : x);
}

void PhysicsObject::Update(float timestep) {

    velocity += acceleration / timestep;
    
    if(ScreenSpaceCollisions) {
        Vector4f MVP_POS = projection.top()*model_view.top()*Vector4f(position[0], position[1], position[2], 1.0);
        int x = floor(displayWidth * ((1.0 + MVP_POS(0) / MVP_POS(3)) / 2.0));
        int y = floor(displayHeight * ((1.0 + MVP_POS(1) / MVP_POS(3)) / 2.0));
        
        uint8_t * geometry = pipeline->RayTracePixel(x, y, true);
        float depth = geometry[3] / 256.0f;
        delete[] geometry;
        
        if(depth < MVP_POS(2) / MVP_POS(3) + INTERSECTION_ADD) {
            velocity = -velocity;
        }    
    }
    
    for(int i = 0; i < 3; i++) {
        velocity(i) = clamp(velocity(i), -MAX_VELOCITY, MAX_VELOCITY);
    }
    
    position += velocity / timestep;
    
}


