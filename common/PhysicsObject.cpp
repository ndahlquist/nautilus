//  PhysicsObject.cpp
//  nativeGraphics

#include "PhysicsObject.h"
#include "common.h"
#include "log.h"

PhysicsObject::PhysicsObject(const char *objFilename, const char *vertexShaderFilename, const char *fragmentShaderFilename)
                                                  : RenderObject(objFilename, vertexShaderFilename, fragmentShaderFilename)  {
    position = Vector3f(0, 0, 0);
    velocity = Vector3f(0, 0, 0);
    acceleration = Vector3f(0, 0, 0);
}

void PhysicsObject::Update(float timestep) {
    position += velocity / timestep;
    velocity += acceleration / timestep;
}


