#ifndef __PARTICLE_SYSTEM_H
#define __PARTICLE_SYSTEM_H

#define SIZE 15

#define CELL_WIDTH 0.06
#define DIRECTION_X 0
#define DIRECTION_Y 1
#define KCFL 0.9
#define FRAME_TIME 0.002
#define SOLID_WALL 1
#define AIR 3
#define FLUID 4
#define GRAVITY 100
#define i_low 7
#define i_high 14
#define j_low 1
#define j_high 15

#include <iostream>
#include <list>
#include "Eigen/Core"
#include "RenderObject.h"

using Eigen::Vector3f;
using Eigen::Vector2f;
using std::list;

class Fluid
{
public:
	double p[SIZE][SIZE];
	double u[SIZE+1][SIZE+1];
	double v[SIZE+1][SIZE+1];
	
  double newu[SIZE+1][SIZE+1];
	double newv[SIZE+1][SIZE+1];
	int status[SIZE][SIZE];
	int layer[SIZE][SIZE];

    RenderObject* renderer;
    list<Vector2f> listParticles;
	double particleRadius;
	double liquid_phi[SIZE+1][SIZE+1];
	
	double deltaTime;
	double remainderTime;
	double maxVelocity;
	Fluid();
	Vector2f getVelocity(double x,double y);
	double getInterpolatedValue(double x, double y, int direction);
	void updateDeltaTime();
	void updateCells();
	void advection();
	void addGravity();
	void getPressure();
	void updateStep();
	void update();
	void moveParticles(float);
    void RenderSetup();
    void RenderFrame();
    float* GenVertexArray();
	Vector2f traceParticle(double x, double y, double t);

};
#endif

