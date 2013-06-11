#pragma once
#include "Cell.h"
#include <cmath>
#include <vector>
#include <list>
#include "Eigen/Sparse"
#include <iostream>

using std::list;
using std::vector;
using Eigen::Vector3f;
using namespace std;

typedef Eigen::Triplet<double> T;
#define Cell_NUM_X 30
#define Cell_NUM_Y 30
#define Cell_NUM_Z 30
#define KCFL 0.7f
#define BUFFER 1
#define DIRECTION_X 0
#define DIRECTION_Y 1
#define DIRECTION_Z 2
#define GRAVITY 10.0f

class Fluid
{
private:
    
    float u[Cell_NUM_X+2*BUFFER][Cell_NUM_Y+2*BUFFER][Cell_NUM_Z+2*BUFFER];
    float v[Cell_NUM_X+2*BUFFER][Cell_NUM_Y+2*BUFFER][Cell_NUM_Z+2*BUFFER];
    float w[Cell_NUM_X+2*BUFFER][Cell_NUM_Y+2*BUFFER][Cell_NUM_Z+2*BUFFER];
    float nu[Cell_NUM_X+2*BUFFER][Cell_NUM_Y+2*BUFFER][Cell_NUM_Z+2*BUFFER];
    float nv[Cell_NUM_X+2*BUFFER][Cell_NUM_Y+2*BUFFER][Cell_NUM_Z+2*BUFFER];
    float nw[Cell_NUM_X+2*BUFFER][Cell_NUM_Y+2*BUFFER][Cell_NUM_Z+2*BUFFER];
    int status[Cell_NUM_X+2*BUFFER][Cell_NUM_Y+2*BUFFER][Cell_NUM_Z+2*BUFFER];
    int layer[Cell_NUM_X][Cell_NUM_Y][Cell_NUM_Z];
    float p[Cell_NUM_X][Cell_NUM_Y][Cell_NUM_Z];
    
	int frameCount;
	float maxVelocity;
	float deltaTime;
    
	float divVelocity(int,int,int);
	Vector3f traceParticle(float x, float y, float z, float t);
	Vector3f getVelocity(float x, float y, float z);
	float getInterpolatedValue(float x, float y, float z, int direction);

    
	void UpdateDeltaTime();
	void UpdateBoundary();
	void UpdateCells();
	void ApplyAdvection();
	void ApplyGravity();
	void ApplyPressure();
	void MoveParticles(float time);
    void AddSource();
    
    //mobile
    //RenderObject* renderer;
    void RenderSetup();
    float* GenVertexArray();
    void Surface(TRIANGLE*&, int&);
    
public:
    Fluid();
	list<Vector3f*> listParticles;
    void Update();
    void RenderFrame();
    
};

Fluid::Fluid(){
    frameCount = 0;
    maxVelocity = 10.f;
    for (int i=0;i<Cell_NUM_X+BUFFER*2;i++)
        for (int j=0; j<Cell_NUM_Y+BUFFER*2; j++)
            for (int k=0; k<Cell_NUM_Z+BUFFER*2; k++) {
                u[i][j][k] = 0.f;
                v[i][j][k] = 0.f;
                w[i][j][k] = 0.f;
                status[i][j][k] = AIR;
    }
    //Initialize solid cells
    for (int i=0;i<Cell_NUM_X+BUFFER*2;i++)
        for (int j=0; j<Cell_NUM_Y+BUFFER*2; j++){
                status[i][j][0] = SOLID;
                //status[i][j][Cell_NUM_Z+BUFFER*2-1] = AIR;
    }
    
    for (int i=0;i<Cell_NUM_X+BUFFER*2;i++)
        for (int k=0; k<Cell_NUM_Z+BUFFER*2; k++){
            status[i][0][k] = SOLID;
            status[i][Cell_NUM_Y+BUFFER*2-1][k] = SOLID;
        }
    
    for (int j=0;j<Cell_NUM_Y+BUFFER*2;j++)
        for (int k=0; k<Cell_NUM_Z+BUFFER*2; k++){
            status[0][j][k] = SOURCE;
            status[Cell_NUM_X+BUFFER*2-1][j][k] = SOLID;
        }
    
    UpdateDeltaTime();
    AddSource();
    UpdateCells();
    UpdateBoundary();
    
    MoveParticles(deltaTime);
    
    //renderer = new RenderObject("standard_v.glsl", "depth_f.glsl");
}


//trace a particle at a point(x,y,z) for t time
Vector3f Fluid::traceParticle(float x, float y, float z, float t)
{
	Vector3f vec = getVelocity(x, y, z);
	vec = getVelocity(x - 0.5f * t * vec[0], y - 0.5f * t * vec[1], z - 0.5f * t * vec[2]);
	return Vector3f(x, y, z) - vec * t;
}

//get the velocity at position(x,y,z)
Vector3f Fluid::getVelocity(float x, float y, float z)
{
	Vector3f vec;
	vec[0] = getInterpolatedValue(x / CELL_WIDTH, y / CELL_WIDTH - 0.5f, z / CELL_WIDTH - 0.5f, DIRECTION_X);
	vec[1] = getInterpolatedValue(x / CELL_WIDTH - 0.5f, y / CELL_WIDTH, z / CELL_WIDTH - 0.5f, DIRECTION_Y);
	vec[2] = getInterpolatedValue(x / CELL_WIDTH - 0.5f, y / CELL_WIDTH - 0.5f, z / CELL_WIDTH, DIRECTION_Z);
	return vec;
}

//get an interpolated value from the grid
float Fluid::getInterpolatedValue(float x, float y, float z, int direction)
{
	int i = (int)(floor(x));
	int j = (int)(floor(y));
	int k = (int)(floor(z));

    
	float weight = 0.0f;
	float sum = 0.0f;
	switch(direction)
	{
        case DIRECTION_X:
           
                sum += (i + 1 - x) * (j + 1 - y) * (k + 1 - z) * u[BUFFER + i][BUFFER + j][BUFFER + k];
                weight += (i + 1 - x) * (j + 1 - y) * (k + 1 - z);
            
                sum += (x - i) * (j + 1 - y) * (k + 1 - z) * u[BUFFER + i + 1][BUFFER + j][BUFFER + k];
                weight += (x - i) * (j + 1 - y) * (k + 1 - z);
           
                sum += (i + 1 - x) * (y - j) * (k + 1 - z) * u[BUFFER + i][BUFFER + j + 1][BUFFER + k];
                weight += (i + 1 - x) * (y - j) * (k + 1 - z);
           
                sum += (x - i) * (y - j) * (k + 1 - z) * u[BUFFER + i + 1][BUFFER + j + 1][BUFFER + k];
                weight += (x - i) * (y - j) * (k + 1 - z);
            
                sum += (i + 1 - x) * (j + 1 - y) * (z - k) * u[BUFFER + i][BUFFER + j][BUFFER + k + 1];
                weight += (i + 1 - x) * (j + 1 - y) * (z - k);
           
                sum += (x - i) * (j + 1 - y) * (z - k) * u[BUFFER + i + 1][BUFFER + j][BUFFER + k + 1];
                weight += (x - i) * (j + 1 - y) * (z - k);
            
                sum += (i + 1 - x) * (y - j) * (z - k) * u[BUFFER + i][BUFFER + j + 1][BUFFER + k + 1];
                weight += (i + 1 - x) * (y - j) * (z - k);
            
                sum += (x - i) * (y - j) * (z - k) * u[BUFFER + i + 1][BUFFER + j + 1][BUFFER + k + 1];
                weight += (x - i) * (y - j) * (z - k);
            
            if(weight)
              return sum / weight;
            break;
        case DIRECTION_Y:
            
            
                sum += (i + 1 - x) * (j + 1 - y) * (k + 1 - z) * v[BUFFER + i][BUFFER + j][BUFFER + k];
                weight += (i + 1 - x) * (j + 1 - y) * (k + 1 - z);
           
                sum += (x - i) * (j + 1 - y) * (k + 1 - z) * v[BUFFER + i + 1][BUFFER + j][BUFFER + k];
                weight += (x - i) * (j + 1 - y) * (k + 1 - z);
            
                sum += (i + 1 - x) * (y - j) * (k + 1 - z) * v[BUFFER + i][BUFFER + j + 1][BUFFER + k];
                weight += (i + 1 - x) * (y - j) * (k + 1 - z);
           
                sum += (x - i) * (y - j) * (k + 1 - z) * v[BUFFER + i + 1][BUFFER + j + 1][BUFFER + k];
                weight += (x - i) * (y - j) * (k + 1 - z);
           
                sum += (i + 1 - x) * (j + 1 - y) * (z - k) * v[BUFFER + i][BUFFER + j][BUFFER + k + 1];
                weight += (i + 1 - x) * (j + 1 - y) * (z - k);
           
                sum += (x - i) * (j + 1 - y) * (z - k) * v[BUFFER + i + 1][BUFFER + j][BUFFER + k + 1];
                weight += (x - i) * (j + 1 - y) * (z - k);
            
                sum += (i + 1 - x) * (y - j) * (z - k) * v[BUFFER + i][BUFFER + j + 1][BUFFER + k + 1];
                weight += (i + 1 - x) * (y - j) * (z - k);
            
                sum += (x - i) * (y - j) * (z - k) * v[BUFFER + i + 1][BUFFER + j + 1][BUFFER + k + 1];
                weight += (x - i) * (y - j) * (z - k);
            
            if(weight)
                return sum / weight;
            break;
        case DIRECTION_Z:
            
                sum += (i + 1 - x) * (j + 1 - y) * (k + 1 - z) * w[BUFFER + i][BUFFER + j][BUFFER + k];
                weight += (i + 1 - x) * (j + 1 - y) * (k + 1 - z);
            
                sum += (x - i) * (j + 1 - y) * (k + 1 - z) * w[BUFFER + i + 1][BUFFER + j][BUFFER + k];
                weight += (x - i) * (j + 1 - y) * (k + 1 - z);
            
                sum += (i + 1 - x) * (y - j) * (k + 1 - z) * w[BUFFER + i][BUFFER + j + 1][BUFFER + k];
                weight += (i + 1 - x) * (y - j) * (k + 1 - z);
           
                sum += (x - i) * (y - j) * (k + 1 - z) * w[BUFFER + i + 1][BUFFER + j + 1][BUFFER + k];
                weight += (x - i) * (y - j) * (k + 1 - z);
            
                sum += (i + 1 - x) * (j + 1 - y) * (z - k) * w[BUFFER + i][BUFFER + j][BUFFER + k + 1];
                weight += (i + 1 - x) * (j + 1 - y) * (z - k);
            
                sum += (x - i) * (j + 1 - y) * (z - k) * w[BUFFER + i + 1][BUFFER + j][BUFFER + k + 1];
                weight += (x - i) * (j + 1 - y) * (z - k);
           
                sum += (i + 1 - x) * (y - j) * (z - k) * w[BUFFER + i][BUFFER + j + 1][BUFFER + k + 1];
                weight += (i + 1 - x) * (y - j) * (z - k);
            
                sum += (x - i) * (y - j) * (z - k) * w[BUFFER + i + 1][BUFFER + j + 1][BUFFER + k + 1];
                weight += (x - i) * (y - j) * (z - k);

            if(weight)
                return sum / weight;
            break;
        default:
            return 0;
	}
    return 0;
}




//one update
void Fluid::Update()
{
    
       
        UpdateDeltaTime();
        if(frameCount<600)
            AddSource();
     
        UpdateCells();
        ApplyAdvection();
        ApplyGravity();
        ApplyPressure();
        UpdateBoundary();
    
        MoveParticles(deltaTime);
    
    
        frameCount++;
}

//calculate the simulation time step
void Fluid::UpdateDeltaTime()
{
	deltaTime = KCFL * CELL_WIDTH / maxVelocity;
    if (deltaTime > 0.05)
        deltaTime = 0.05;
    
}


//update the grid based on the marker particles
void Fluid::UpdateCells()
{
    for (int i=0;i<Cell_NUM_X;i++)
        for (int j=0; j<Cell_NUM_Y; j++)
            for (int k=0; k<Cell_NUM_Z; k++) {
               layer[i][j][k]=-1;
    }

	for (list<Vector3f*>::iterator iter = listParticles.begin(); iter != listParticles.end();)
	{
        int i=(int)floor((**iter)[0]/CELL_WIDTH);
        int j=(int)floor((**iter)[1]/CELL_WIDTH);
        int k=(int)floor((**iter)[2]/CELL_WIDTH);
        
        int posX = BUFFER+i;
        int posY = BUFFER+j;
        int posZ = BUFFER+k;
        
        if (status[posX][posY][posZ]!=SOLID && status[posX][posY][posZ]!= SOURCE)
        {
            status[posX][posY][posZ] = FLUID;
            layer[i][j][k] = 0;
            iter++;
        }
		else
		{
			delete(*iter);
			iter = listParticles.erase(iter);
			
		}
	}
    
    for (int i=0;i<Cell_NUM_X;i++)
        for (int j=0; j<Cell_NUM_Y; j++)
            for (int k=0; k<Cell_NUM_Z; k++) {
                
                int posX = BUFFER+i;
                int posY = BUFFER+j;
                int posZ = BUFFER+k;
                                
                if(layer[i][j][k]==-1){
                    status[posX][posY][posZ] = AIR;
                    
                }
        
    }
}

//apply convection using a backwards particle trace
void Fluid::ApplyAdvection()
{
	
    for (int i=0;i<Cell_NUM_X;i++)
        for (int j=0; j<Cell_NUM_Y; j++)
            for (int k=0; k<Cell_NUM_Z; k++) {
                int posX = BUFFER + i;
                int posY = BUFFER + j;
                int posZ = BUFFER + k;
                
                Vector3f vec;
                
                vec = this->traceParticle(i * CELL_WIDTH, (j+0.5f) * CELL_WIDTH , (k+0.5f) * CELL_WIDTH, deltaTime);
                nu[posX][posY][posZ] = this->getVelocity(vec[0], vec[1], vec[2])[0];
                
                vec = this->traceParticle((i+0.5f) * CELL_WIDTH, j * CELL_WIDTH, (k+0.5f) * CELL_WIDTH, deltaTime);
                nv[posX][posY][posZ] = this->getVelocity(vec[0], vec[1], vec[2])[1];
                
                vec = this->traceParticle((i+0.5f) * CELL_WIDTH, (j+0.5f) * CELL_WIDTH, k * CELL_WIDTH, deltaTime);
                nw[posX][posY][posZ] = this->getVelocity(vec[0], vec[1], vec[2])[2];
                                
	}
    
}

//apply gravity(external force)
void Fluid::ApplyGravity()
{
	
    for (int i=0;i<Cell_NUM_X;i++)
        for (int j=0; j<Cell_NUM_Y; j++)
            for (int k=0; k<Cell_NUM_Z; k++) {
                
                int posX = BUFFER + i;
                int posY = BUFFER + j;
                int posZ = BUFFER + k;
                
                u[posX][posY][posZ] = nu[posX][posY][posZ];
                v[posX][posY][posZ] = nv[posX][posY][posZ];
                w[posX][posY][posZ] = nw[posX][posY][posZ];
                
                if(status[posX][posY][posZ] == FLUID || status[posX][posY][posZ-1] == FLUID){
                    v[posX][posY][posZ] -= deltaTime * GRAVITY;
                }
                
		}
}

//calculate the divergence of velocity at the centre of a cell
float Fluid::divVelocity(int posX, int posY, int posZ)
{
    
	float ret = 0.0f;
	if (status[posX - 1][posY][posZ] == FLUID || status[posX - 1][posY][posZ] == AIR)
	{
		ret += u[posX][posY][posZ];
	}
    
	if (status[posX][posY - 1][posZ] == FLUID || status[posX][posY - 1][posZ] == AIR)
	{
		ret += v[posX][posY][posZ];
	}
    
	if (status[posX][posY][posZ - 1] == FLUID || status[posX][posY][posZ - 1] == AIR)
	{
		ret += w[posX][posY][posZ];
	}
    
	if (status[posX+1][posY][posZ] == FLUID || status[posX+1][posY][posZ] == AIR)
	{
		ret -= u[posX+1][posY][posZ];
	}
	
    
	if (status[posX][posY+1][posZ] == FLUID || status[posX][posY+1][posZ] == AIR)
	{
		ret -= v[posX][posY+1][posZ];
	}
	
    
	if (status[posX][posY][posZ+1] == FLUID || status[posX][posY][posZ+1] == AIR)
	{
		ret -= w[posX][posY][posZ+1];
	}
	
	
	return ret;
}

//apply pressure
void Fluid::ApplyPressure()
{
	int count = 0;
	
    for (int i=0;i<Cell_NUM_X;i++)
        for (int j=0; j<Cell_NUM_Y; j++)
            for (int k=0; k<Cell_NUM_Z; k++) {
                
                int posX = BUFFER + i;
                int posY = BUFFER + j;
                int posZ = BUFFER + k;
                
                if (status[posX][posY][posZ] == FLUID){
                    layer[i][j][k] = count;
                    count++;
                }
	}
	Eigen::VectorXd b(count);
	
    
    std::vector<T> tripletList;
    tripletList.reserve(7*count);
   

    //Eigen::SparseMatrix<double> mat(count,count); // default is column major
    //mat.reserve(Eigen::VectorXi::Constant(count,7));
        
	count = 0;
	
    for (int i=0;i<Cell_NUM_X;i++)
        for (int j=0; j<Cell_NUM_Y; j++)
            for (int k=0; k<Cell_NUM_Z; k++) {
                
                int posX = BUFFER + i;
                int posY = BUFFER + j;
                int posZ = BUFFER + k;
                
                if (status[posX][posY][posZ] != FLUID){
                    continue;
                }
                
                b[layer[i][j][k]] = divVelocity(posX,posY,posZ);
                
                
                int neighbor = 0;
        
                if (status[posX - 1][posY][posZ] == FLUID)
                {
                    neighbor++;

                    tripletList.push_back(T(count,layer[i-1][j][k],-1.0));
                }
                else if (status[posX - 1][posY][posZ] == AIR)
                {
                    neighbor++;
                }   
        
                if (status[posX + 1][posY][posZ] == FLUID)
                {
                    neighbor++;
                    
                    tripletList.push_back(T(count,layer[i+1][j][k],-1.0));
                }
                else if (status[posX + 1][posY][posZ] == AIR)
                {
                    neighbor++;
                }
        
                if (status[posX][posY - 1][posZ] == FLUID)
                {
                    neighbor++;
                    
                    tripletList.push_back(T(count,layer[i][j-1][k],-1.0));
                }
                else if (status[posX][posY - 1][posZ] == AIR)
                {
                    neighbor++;
                }
        
                if (status[posX][posY + 1][posZ] == FLUID)
                {
                    neighbor++;
        
                    tripletList.push_back(T(count,layer[i][j+1][k],-1.0));
                }
                else if (status[posX][posY + 1][posZ] == AIR)
                {
                    neighbor++;
                }
        
                if (status[posX][posY][posZ - 1] == FLUID)
                {
                    neighbor++;
                    
                    tripletList.push_back(T(count,layer[i][j][k-1],-1.0));
                }
                else if (status[posX][posY][posZ - 1] == AIR)
                {
                    neighbor++;
                }
        
                if (status[posX][posY][posZ + 1] == FLUID)
                {
                    neighbor++;
                    
                    tripletList.push_back(T(count,layer[i][j][k+1],-1.0));
                }
                else if (status[posX][posY][posZ + 1] == AIR)
                {
                    neighbor++;
                }   
		
                tripletList.push_back(T(count,count,(double)(neighbor)));
                count++;
                
            }
    // Solving:
    Eigen::SparseMatrix<double> mat(count, count);
    mat.setFromTriplets(tripletList.begin(), tripletList.end());
    
    Eigen::ConjugateGradient<Eigen::SparseMatrix<double>> solver(mat); 
    Eigen::VectorXd x = solver.solve(b); // use the factorization to solve for the given right hand side
    
    //update pressure
	for (int i=0;i<Cell_NUM_X;i++)
        for (int j=0; j<Cell_NUM_Y; j++)
            for (int k=0; k<Cell_NUM_Z; k++) {
                
                int posX = BUFFER + i;
                int posY = BUFFER + j;
                int posZ = BUFFER + k;
                
                if (status[posX][posY][posZ] != FLUID){
                    p[i][j][k] = 0;
                    continue;
                }
                p[i][j][k] = x[layer[i][j][k]];

                u[posX][posY][posZ] -= p[i][j][k];
                u[posX+1][posY][posZ] += p[i][j][k];

                v[posX][posY][posZ] -= p[i][j][k];
                v[posX][posY+1][posZ] += p[i][j][k];

                w[posX][posY][posZ] -= p[i][j][k];
                w[posX][posY][posZ+1] += p[i][j][k];
    }

	

	maxVelocity = 0.0f;
	
    for (int i=0;i<Cell_NUM_X+BUFFER;i++)
        for (int j=0; j<Cell_NUM_Y+BUFFER; j++)
            for (int k=0; k<Cell_NUM_Z+BUFFER; k++) {
                
                int posX = BUFFER + i;
                int posY = BUFFER + j;
                int posZ = BUFFER + k;
                
                if(status[posX][posY][posZ] != FLUID)
                    continue;
                
                float l = u[posX][posY][posZ]*u[posX][posY][posZ] + v[posX][posY][posZ]*v[posX][posY][posZ] + w[posX][posY][posZ]*w[posX][posY][posZ];
                
                if (maxVelocity < l)
                {
                    maxVelocity = l;
                }
    }
    maxVelocity = sqrt(maxVelocity);
    
}

//extrapolate the fluid velocity to the buffer zone
void Fluid::UpdateBoundary()//SUPER IMPORTANT!
{
    float a =0.5;
    float u0 = 5.f;
    
    for (int i=0;i<Cell_NUM_X+BUFFER*2;i++)
        for (int j=0; j<Cell_NUM_Y+BUFFER*2; j++){
            w[i][j][1] = 0.f;
            w[i][j][0] = 0.f;
            u[i][j][0] = a*u[i][j][1];
            v[i][j][0] = a*v[i][j][1];
            w[i][j][Cell_NUM_Z+BUFFER*2-1] = 0.f;
            u[i][j][Cell_NUM_Z+BUFFER*2-1] = a*u[i][j][Cell_NUM_Z+BUFFER*2-2];
            v[i][j][Cell_NUM_Z+BUFFER*2-1] = a*v[i][j][Cell_NUM_Z+BUFFER*2-2];
            
        }
    for (int i=0;i<Cell_NUM_X+BUFFER*2;i++)
        for (int k=0; k<Cell_NUM_Y+BUFFER*2; k++){
            v[i][1][k] = 0.f;
            u[i][0][k] = a*u[i][1][k];
            v[i][0][k] = 0.f;
            w[i][0][k] = a*w[i][1][k];
            u[i][Cell_NUM_Y+BUFFER*2-1][k] = a*u[i][Cell_NUM_Y+BUFFER*2-2][k];
            v[i][Cell_NUM_Y+BUFFER*2-1][k] = 0.f;
            w[i][Cell_NUM_Y+BUFFER*2-1][k] = a*w[i][Cell_NUM_Y+BUFFER*2-2][k];
            
        }
    for (int j=0; j<Cell_NUM_Y+BUFFER*2; j++)
        for (int k=0; k<Cell_NUM_Y+BUFFER*2; k++){
            if(frameCount<80){
            u[1][j][k] = u0;
            u[0][j][k] = u0;
            }
            else{
                u[1][j][k] = 0.f;
            u[0][j][k] = 0.f;
            }
            v[0][j][k] = a*v[1][j][k];
            w[0][j][k] = a*w[1][j][k];
            u[Cell_NUM_X+BUFFER*2-1][j][k] = 0.f;
            v[Cell_NUM_X+BUFFER*2-1][j][k] = a*v[Cell_NUM_X +BUFFER*2 -2][j][k];
            w[Cell_NUM_X+BUFFER*2-1][j][k] = a*w[Cell_NUM_X +BUFFER*2 -2][j][k];
            
        }

}



//move particles for time t
void Fluid::MoveParticles(float time)
{
	for (list<Vector3f *>::iterator iter = listParticles.begin(); iter != listParticles.end();)
	{
		Vector3f v = getVelocity((**iter)[0],(**iter)[1], (**iter)[2]);
		
        Vector3f test = **iter + v*time;
        int i = (int)floor(test[0]/CELL_WIDTH);
        int j = (int)floor(test[1]/CELL_WIDTH);
        int k = (int)floor(test[2]/CELL_WIDTH);
        
		if (status[BUFFER+i][BUFFER+j][BUFFER+k] != SOLID)
		{
			**iter = test;
            iter++;
		}
        else{
            //iter++;
            delete (*iter);
            iter = listParticles.erase(iter);
        }
	}
}

void Fluid::AddSource(){
    float y,z;
    for (int j = 0; j < Cell_NUM_Y; j++)
        for (int k = 0; k < Cell_NUM_Z; k++)
        {
            for(int step = 0; step<1; step++){
                y = j+((float) rand()) / (float) RAND_MAX;
                z = k+((float) rand()) / (float) RAND_MAX;
                listParticles.push_back(new Vector3f(0.f , y* CELL_WIDTH, z * CELL_WIDTH));
            }
            
        }
}

float* Fluid::GenVertexArray(){
    float* vertices = new float[6*8*listParticles.size()];
    int bufferIndex = 0;
    float radius = .001;
    for(list<Vector3f*>::iterator iter=listParticles.begin();iter != listParticles.end();iter++){
        // Triangle 1
        vertices[bufferIndex++] =(**iter)[0]-radius;
        vertices[bufferIndex++] =(**iter)[1]-radius;
        vertices[bufferIndex++] =(**iter)[2];
        vertices[bufferIndex++] =(**iter)[0]+radius;
        vertices[bufferIndex++] =(**iter)[1]-radius;
        vertices[bufferIndex++] =(**iter)[2];
        vertices[bufferIndex++] =(**iter)[0]+radius;
        vertices[bufferIndex++] =(**iter)[1]+radius;
        vertices[bufferIndex++] =(**iter)[2];
        
        // Triangle 2
        vertices[bufferIndex++] =(**iter)[0]-radius;
        vertices[bufferIndex++] =(**iter)[1]-radius;
        vertices[bufferIndex++] =(**iter)[2];
        vertices[bufferIndex++] =(**iter)[0]-radius;
        vertices[bufferIndex++] =(**iter)[1]+radius;
        vertices[bufferIndex++] =(**iter)[2];
        vertices[bufferIndex++] =(**iter)[0]+radius;
        vertices[bufferIndex++] =(**iter)[1]+radius;
        vertices[bufferIndex++] =(**iter)[2];
    }
    return vertices;
}

void Fluid::RenderFrame(){
    //Update();
    //renderer->RenderFrame(GenVertexArray(),listParticles.size()*6);
    
    //Choose render surface or points
    //Render Surface
    TRIANGLE *tri = NULL;
	int ntri = 0;
    Surface(tri,ntri);
    glColor3f(0,0,1);
    glBegin(GL_TRIANGLES);
    for (int i=0;i<ntri;i++) {
        for (int k=0;k<3;k++)  {
            glVertex3f(tri[i].p[k].x,tri[i].p[k].y,tri[i].p[k].z);
        }
	}
    glEnd();
    
    //Render Points
    /*glColor3f(0.0,0.0,1.0);
    
	glPointSize(3.0);
    glBegin(GL_POINTS);
	for(list<Vector3f*>::iterator iter=listParticles.begin();iter != listParticles.end();iter++){
		
        glVertex3f((**iter)[0],(**iter)[1],(**iter)[2]);
		
    }
    glEnd();*/
    
}


void Fluid::Surface(TRIANGLE*& tri, int& ntri){
    //vertex grid store signed distance
    int R=1;
    mcCell mcgrid[BUFFER*2+R*Cell_NUM_X][BUFFER*2+R*Cell_NUM_Y][BUFFER*2+R*Cell_NUM_Z];
    for (int i=0;i<Cell_NUM_X*R+BUFFER*2;i++) {
		for (int j=0;j<Cell_NUM_Y*R+BUFFER*2;j++) {
			for (int k=0;k<Cell_NUM_Z*R+BUFFER*2;k++) {
                //mcgrid[BUFFER+i][BUFFER+j][BUFFER+k].m=100;
                mcgrid[i][j][k].visited = false;
            }
        }
    }
    //GRIDCELL mcgrid;
    //iterate through particles, locally update
    for(list<Vector3f*>::iterator iter=listParticles.begin();iter != listParticles.end();iter++){
        float px = (**iter)[0], py = (**iter)[1], pz = (**iter)[2];
        int i0 = static_cast<int>(floor(px/CELL_WIDTH))*R;
        int j0 = static_cast<int>(floor(py/CELL_WIDTH))*R;
        int k0 = static_cast<int>(floor(pz/CELL_WIDTH))*R;
        
        float x0 = i0*CELL_WIDTH/R;
        float y0 = j0*CELL_WIDTH/R;
        float z0 = k0*CELL_WIDTH/R;
        
        i0+=BUFFER;
        j0+=BUFFER;
        k0+=BUFFER;
        for(int r=0;r<R;r++)
            for(int s=0;s<R;s++)
                for(int t=0;t<R;t++){
                    int i=i0+r;
                    int j=j0+s;
                    int k=k0+t;
                    float x=x0+r*CELL_WIDTH/R, y= y0+s*CELL_WIDTH/R, z = z0+t*CELL_WIDTH/R;
        //i,j,k
        float d = sqrt((x-px)*(x-px)+(y-py)*(y-py)+(z-pz)*(z-pz));
        //printf("%f %f %f %f %f %f %f\n",x,px,y,py,z,pz,d);
        if (!mcgrid[i][j][k].visited){
            mcgrid[i][j][k].visited = true;
            mcgrid[i][j][k].max = d;
            mcgrid[i][j][k].min = d;
        }
        else{
            mcgrid[i][j][k].min =  d<mcgrid[i][j][k].min?d:mcgrid[i][j][k].min;
            mcgrid[i][j][k].max =  d>mcgrid[i][j][k].max?d:mcgrid[i][j][k].max;

        }
        
        //i+1,j,k
        d = sqrt((x+CELL_WIDTH/R-px)*(x+CELL_WIDTH/R-px)+(y-py)*(y-py)+(z-pz)*(z-pz));
        if (!mcgrid[i+1][j][k].visited){
            mcgrid[i+1][j][k].visited = true;
            mcgrid[i+1][j][k].max = d;
            mcgrid[i+1][j][k].min = d;
        }
        else{
            mcgrid[i+1][j][k].min =  d<mcgrid[i+1][j][k].min?d:mcgrid[i+1][j][k].min;
            mcgrid[i+1][j][k].max =  d>mcgrid[i+1][j][k].max?d:mcgrid[i+1][j][k].max;
        }
        
        //i,j+1,k
        d = sqrt((x-px)*(x-px)+(y+CELL_WIDTH/R-py)*(y+CELL_WIDTH/R-py)+(z-pz)*(z-pz));
        if (!mcgrid[i][j+1][k].visited){
            mcgrid[i][j+1][k].visited = true;
            mcgrid[i][j+1][k].max = d;
            mcgrid[i][j+1][k].min = d;
        }
        else{
            mcgrid[i][j+1][k].min =  d<mcgrid[i][j+1][k].min?d:mcgrid[i][j+1][k].min;
            mcgrid[i][j+1][k].max =  d>mcgrid[i][j+1][k].max?d:mcgrid[i][j+1][k].max;
        }
        
        //i+1,j+1,k
        d = sqrt((x+CELL_WIDTH/R-px)*(x+CELL_WIDTH/R-px)+(y+CELL_WIDTH/R-py)*(y+CELL_WIDTH/R-py)+(z-pz)*(z-pz));
        if (!mcgrid[i+1][j+1][k].visited){
            mcgrid[i+1][j+1][k].visited = true;
            mcgrid[i+1][j+1][k].max = d;
            mcgrid[i+1][j+1][k].min = d;
        }
        else{
            mcgrid[i+1][j+1][k].min =  d<mcgrid[i+1][j+1][k].min?d:mcgrid[i+1][j+1][k].min;
            mcgrid[i+1][j+1][k].max =  d>mcgrid[i+1][j+1][k].max?d:mcgrid[i+1][j+1][k].max;
        }
        
        //i,j,k+1
        d = sqrt((x-px)*(x-px)+(y-py)*(y-py)+(z+CELL_WIDTH/R-pz)*(z+CELL_WIDTH/R-pz));
        if (!mcgrid[i][j][k+1].visited){
            mcgrid[i][j][k+1].visited = true;
            mcgrid[i][j][k+1].max = d;
            mcgrid[i][j][k+1].min = d;
        }
        else{
            mcgrid[i][j][k+1].min =  d<mcgrid[i][j][k+1].min?d:mcgrid[i][j][k+1].min;
            mcgrid[i][j][k+1].max =  d>mcgrid[i][j][k+1].max?d:mcgrid[i][j][k+1].max;
        }
        
        //i+1,j,k+1
        d = sqrt((x+CELL_WIDTH/R-px)*(x+CELL_WIDTH/R-px)+(y-py)*(y-py)+(z+CELL_WIDTH/R-pz)*(z+CELL_WIDTH/R-pz));
        if (!mcgrid[i+1][j][k+1].visited){
            mcgrid[i+1][j][k+1].visited = true;
            mcgrid[i+1][j][k+1].max = d;
            mcgrid[i+1][j][k+1].min = d;
        }
        else{
            mcgrid[i+1][j][k+1].min =  d<mcgrid[i+1][j][k+1].min?d:mcgrid[i+1][j][k+1].min;
            mcgrid[i+1][j][k+1].max =  d>mcgrid[i+1][j][k+1].max?d:mcgrid[i+1][j][k+1].max;
        }
        
        //i,j+1,k+1
        d = sqrt((x-px)*(x-px)+(y+CELL_WIDTH/R-py)*(y+CELL_WIDTH/R-py)+(z+CELL_WIDTH/R-pz)*(z+CELL_WIDTH/R-pz));
        if (!mcgrid[i][j+1][k+1].visited){
            mcgrid[i][j+1][k+1].visited = true;
            mcgrid[i][j+1][k+1].max = d;
            mcgrid[i][j+1][k+1].min = d;
        }
        else{
            mcgrid[i][j+1][k+1].min =  d<mcgrid[i][j+1][k+1].min?d:mcgrid[i][j+1][k+1].min;
            mcgrid[i][j+1][k+1].max =  d>mcgrid[i][j+1][k+1].max?d:mcgrid[i][j+1][k+1].max;
        }
        
        //i+1,j+1,k+1
        d = sqrt((x+CELL_WIDTH/R-px)*(x+CELL_WIDTH/R-px)+(y+CELL_WIDTH/R-py)*(y+CELL_WIDTH/R-py)+(z+CELL_WIDTH/R-pz)*(z+CELL_WIDTH/R-pz));
        if (!mcgrid[i+1][j+1][k+1].visited){
            mcgrid[i+1][j+1][k+1].visited = true;
            mcgrid[i+1][j+1][k+1].max = d;
            mcgrid[i+1][j+1][k+1].min = d;
        }
        else{
            mcgrid[i+1][j+1][k+1].min =  d<mcgrid[i+1][j+1][k+1].min?d:mcgrid[i+1][j+1][k+1].min;
            mcgrid[i+1][j+1][k+1].max =  d>mcgrid[i+1][j+1][k+1].max?d:mcgrid[i+1][j+1][k+1].max;
        }
                }
    }
    GRIDCELL Grid;
    TRIANGLE triangles[10];
	
    int n=0;
    float radius=CELL_WIDTH/R;
    //iterate through all cells and construct triangle mesh
    // Polygonise the grid
	fprintf(stderr,"Polygonising data ...\n");
	
    for (int i_=0;i_<Cell_NUM_X;i_++)
        for (int j_=0; j_<Cell_NUM_Y; j_++)
            for (int k_=0; k_<Cell_NUM_Z; k_++) {
    
		//Cell *g = *iter;
		int i0 = BUFFER + i_*R;
		int j0 = BUFFER + j_*R;
		int k0 = BUFFER + k_*R;
        if(status[BUFFER+i_][BUFFER+j_][BUFFER+k_]!=FLUID && status[BUFFER+i_][BUFFER+j_][BUFFER+k_]!=AIR)
        continue;
        /*if(grid[i0+1][j0][k0].status==FLUID && (grid[i0-1][j0][k0].status!=AIR
           && grid[i0][j0+1][k0].status!=AIR && grid[i0][j0-1][k0].status!=AIR
           && grid[i0][j0][k0+1].status!=AIR && grid[i0][j0][k0-1].status!=AIR))
            continue;*/
        //if(g->x==0||g->x==Cell_NUM_X-1||g->y==0||g->y==Cell_NUM_Y-1||g->z==0||g->z==Cell_NUM_Z-1)
        //  continue;

        for(int r=0;r<R;r++)
            for(int s=0;s<R;s++)
                for(int t=0;t<R;t++){
                    int i=i0+r;
                    int j=j0+s;
                    int k=k0+t;
                float x=i*CELL_WIDTH/R, y= j*CELL_WIDTH/R, z = k*CELL_WIDTH/R;
                //vertex 0: i,j,k
				Grid.p[0].x = x;
                Grid.p[0].y = y;
                Grid.p[0].z = z;
                if(mcgrid[i][j][k].visited)
				  Grid.val[0] = mcgrid[i][j][k].min<radius?-mcgrid[i][j][k].max:mcgrid[i][j][k].min;
                else
                  Grid.val[0] = CELL_WIDTH;
                //1:i+1,j,k
                Grid.p[1].x = x+CELL_WIDTH/R;
                Grid.p[1].y = y;
                Grid.p[1].z = z;
                if(mcgrid[i+1][j][k].visited)
                    Grid.val[1] = mcgrid[i+1][j][k].min<radius?-mcgrid[i+1][j][k].max:mcgrid[i+1][j][k].min;
                else
                    Grid.val[1] = CELL_WIDTH;
                //2:i+1,j+1,k
                Grid.p[2].x = x+CELL_WIDTH/R;
                Grid.p[2].y = y+CELL_WIDTH/R;
                Grid.p[2].z = z;
                if(mcgrid[i+1][j+1][k].visited)
                    Grid.val[2] = mcgrid[i+1][j+1][k].min<radius?-mcgrid[i+1][j+1][k].max:mcgrid[i+1][j+1][k].min;
                else
                    Grid.val[2] = CELL_WIDTH;
                //3:i,j+1,k
                Grid.p[3].x = x;
                Grid.p[3].y = y+CELL_WIDTH/R;
                Grid.p[3].z = z;
                if(mcgrid[i][j+1][k].visited)
                    Grid.val[3] = mcgrid[i][j+1][k].min<radius?-mcgrid[i][j+1][k].max:mcgrid[i][j+1][k].min;
                else
                    Grid.val[3] = CELL_WIDTH;
                //4:i,j,k+1
                Grid.p[4].x = x;
                Grid.p[4].y = y;
                Grid.p[4].z = z+CELL_WIDTH/R;
                if(mcgrid[i][j][k+1].visited)
                    Grid.val[4] = mcgrid[i][j][k+1].min<radius?-mcgrid[i][j][k+1].max:mcgrid[i][j][k+1].min;
                else
                    Grid.val[4] = CELL_WIDTH;
                //5:i+1,j,k+1
                Grid.p[5].x = x+CELL_WIDTH/R;
                Grid.p[5].y = y;
                Grid.p[5].z = z+CELL_WIDTH/R;
                if(mcgrid[i+1][j][k+1].visited)
                    Grid.val[5] = mcgrid[i+1][j][k+1].min<radius?-mcgrid[i+1][j][k+1].max:mcgrid[i+1][j][k+1].min;
                else
                    Grid.val[5] = CELL_WIDTH;
                //6:i+1,j+1,k+1
                Grid.p[6].x = x+CELL_WIDTH/R;
                Grid.p[6].y = y+CELL_WIDTH/R;
                Grid.p[6].z = z+CELL_WIDTH/R;
                if(mcgrid[i+1][j+1][k+1].visited)
				  Grid.val[6] = mcgrid[i+1][j+1][k+1].min<radius?-mcgrid[i+1][j+1][k+1].max:mcgrid[i+1][j+1][k+1].min;
                else
                    Grid.val[6] = CELL_WIDTH;
                //7:i,j+1,k+1
                Grid.p[7].x = x;
                Grid.p[7].y = y+CELL_WIDTH/R;
                Grid.p[7].z = z+CELL_WIDTH/R;
                if(mcgrid[i][j+1][k+1].visited)
				    Grid.val[7] = mcgrid[i][j+1][k+1].min<radius?-mcgrid[i][j+1][k+1].max:mcgrid[i][j+1][k+1].min;
				else
                    Grid.val[7] = CELL_WIDTH;
                

				n = PolygoniseCube(Grid,0.0,triangles);
				tri = (TRIANGLE*)realloc(tri,(ntri+n)*sizeof(TRIANGLE));
				for (int l=0;l<n;l++)
					tri[ntri+l] = triangles[l];
				ntri += n;
	    }
    }

    
    
    //calculate the signed distance value for the cell
}