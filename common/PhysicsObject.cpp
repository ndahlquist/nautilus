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

#define MAX_VELOCITY 1.0
#define COEFF_RESTITUTION .85f

PhysicsObject::PhysicsObject(const char *objFilename, const char *vertexShaderFilename, const char *fragmentShaderFilename)
                                                  : RenderObject(objFilename, vertexShaderFilename, fragmentShaderFilename)  {
    position = Vector3f(0, 0, 0);
    velocity = Vector3f(0, 0, 0);
    acceleration = Vector3f(0, -.0001, 0);
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
        uint8_t depth = geometry[3];
        Vector4f MV_normal = Vector4f(geometry[0] / 128.0f - 1.0f, geometry[1] / 128.0f - 1.0f, geometry[2] / 128.0f - 1.0f, 0.0);
        
        Vector4f normal_hom = model_view.top().inverse() * MV_normal;
        Vector3f normal = Vector3f(normal_hom(0), -normal_hom(1), normal_hom(2)); // TODO: Why is this negative??
        normal.normalize();
        
        delete[] geometry;
        
        if(depth < 256 * MVP_POS(2) / MVP_POS(3))
            velocity = COEFF_RESTITUTION * (-2 * velocity.dot(normal) * normal + velocity);
    }
    
    for(int i = 0; i < 3; i++)
        velocity(i) = clamp(velocity(i), -MAX_VELOCITY, MAX_VELOCITY);
    
    position += velocity / timestep;
    
}


