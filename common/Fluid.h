#pragma once
#include "Cell.h"
#include "RenderObject.h"
#include "log.h"
#include <cmath>
#include <vector>
#include <list>
#include "Eigen/Sparse"

using std::list;
using std::vector;
using Eigen::Vector3f;

typedef Eigen::Triplet<double> T;
#define Cell_NUM_X 3
#define Cell_NUM_Y 6
#define Cell_NUM_Z 6
#define KCFL 0.7f
#define BUFFER 1
#define DIRECTION_X 0
#define DIRECTION_Y 1
#define DIRECTION_Z 2
#define GRAVITY -2.f
#define FRAME_TIME 0.04f

class Fluid : public RenderObject {
public:
    Fluid(const char *vertexShaderFilename, const char *fragmentShaderFilename);
	list<struct Particle*> listParticles;
    void Update();
    void Render(float rx,float ry, float rz);

private:
    void RenderPass(int instance, GLfloat *buffer, int num);

    float u[Cell_NUM_X+2*BUFFER][Cell_NUM_Y+2*BUFFER][Cell_NUM_Z+2*BUFFER];
    float v[Cell_NUM_X+2*BUFFER][Cell_NUM_Y+2*BUFFER][Cell_NUM_Z+2*BUFFER];
    float w[Cell_NUM_X+2*BUFFER][Cell_NUM_Y+2*BUFFER][Cell_NUM_Z+2*BUFFER];
    float nu[Cell_NUM_X+2*BUFFER][Cell_NUM_Y+2*BUFFER][Cell_NUM_Z+2*BUFFER];
    float nv[Cell_NUM_X+2*BUFFER][Cell_NUM_Y+2*BUFFER][Cell_NUM_Z+2*BUFFER];
    float nw[Cell_NUM_X+2*BUFFER][Cell_NUM_Y+2*BUFFER][Cell_NUM_Z+2*BUFFER];
    int status[Cell_NUM_X+2*BUFFER][Cell_NUM_Y+2*BUFFER][Cell_NUM_Z+2*BUFFER];
    int layer[Cell_NUM_X][Cell_NUM_Y][Cell_NUM_Z];
    float p[Cell_NUM_X][Cell_NUM_Y][Cell_NUM_Z];
    Eigen::Matrix4f rot;
    
	int frameCount;
	float maxVelocity;
	float deltaTime;
	float remainderTime;
    
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
    void Rotate(float, float, float);
    
    //mobile
    RenderObject* renderer;
    float* GenVertexArrayInBound(int&);
    float* GenVertexArrayOutBound(int&);
    float* Surface(TRIANGLE*&, int&);
};

Fluid::Fluid(const char *vertexShaderFilename, const char *fragmentShaderFilename)
           : RenderObject(vertexShaderFilename, fragmentShaderFilename, true) {
    frameCount = 0;
    remainderTime = 0.f;
    maxVelocity = 100.f;
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
            status[i][j][Cell_NUM_Z+BUFFER*2-1] = SOLID;
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




//update the status of fluid
void Fluid::Update()
{
    if (remainderTime >= FRAME_TIME)
    {
        //if(frameCount<600 && frameCount%10 == 0)
            AddSource();
        remainderTime -= FRAME_TIME;
        MoveParticles(FRAME_TIME);
    }
    else{
        if(remainderTime){
            MoveParticles(remainderTime);
        }
        UpdateDeltaTime();		//1
        if(deltaTime <= FRAME_TIME){
            remainderTime = 0.f;
        }
        else{
            deltaTime = FRAME_TIME;
            remainderTime = deltaTime - FRAME_TIME;
        }
        //if(frameCount<60)
            AddSource();
        UpdateCells();			//2
        ApplyAdvection();		//3a
        ApplyGravity();			//3b
        ApplyPressure();		//3de
        UpdateBoundary();	//3f
        
        MoveParticles(deltaTime);
        
    }
    
    //frameCount++;
}

//calculate the simulation time step
void Fluid::UpdateDeltaTime()
{
	deltaTime = KCFL * CELL_WIDTH / maxVelocity;

}

//update the grid based on the marker particles
void Fluid::UpdateCells()
{
    for (int i=0;i<Cell_NUM_X;i++)
        for (int j=0; j<Cell_NUM_Y; j++)
            for (int k=0; k<Cell_NUM_Z; k++) {
                layer[i][j][k]=-1;
            }
    
	for (list<struct Particle*>::iterator iter = listParticles.begin(); iter != listParticles.end();)
	{
        if (!(*iter)->inBound) {
            if(!(*iter)->life){
                delete (*iter);
                iter = listParticles.erase(iter);
                continue;
            }
            (*iter)->life--;
            iter++;
        }
        else{
        
        int i=(int)floor((*iter)->pos[0]/CELL_WIDTH);
        int j=(int)floor((*iter)->pos[1]/CELL_WIDTH);
        int k=(int)floor((*iter)->pos[2]/CELL_WIDTH);
        
        int posX = BUFFER+i;
        int posY = BUFFER+j;
        int posZ = BUFFER+k;
        
        if(posX<0||posX>Cell_NUM_X+2*BUFFER-1||posY<0||posY>Cell_NUM_Y+2*BUFFER-1||posZ<0||posZ>Cell_NUM_Z+2*BUFFER-1){
            (*iter)->inBound = false;
            iter++;
            continue;
        }
        
        if (status[posX][posY][posZ]!=SOLID && status[posX][posY][posZ]!= SOURCE)
        {
            status[posX][posY][posZ] = FLUID;
            layer[i][j][k] = 0;
            iter++;
        }
		else if (status[posX][posY][posZ]==SOLID)
		{
            (*iter)->inBound = false;
            iter++;
		}
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
                
                //printf("%f %f %f\n", nu[posX][posY][posZ],nv[posX][posY][posZ],nw[posX][posY][posZ]);
                
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
                
                // printf("%f %f %f\n", u[posX][posY][posZ],v[posX][posY][posZ],w[posX][posY][posZ]);
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
                    //mat.insert(count,layer[i-1][j][k]) = -1.f;
                    tripletList.push_back(T(count,layer[i-1][j][k],-1.0));
                }
                else if (status[posX - 1][posY][posZ] == AIR)
                {
                    neighbor++;
                }
                
                if (status[posX + 1][posY][posZ] == FLUID)
                {
                    neighbor++;
                    //mat.insert(count,layer[i+1][j][k]) = -1.f;
                    tripletList.push_back(T(count,layer[i+1][j][k],-1.0));
                }
                else if (status[posX + 1][posY][posZ] == AIR)
                {
                    neighbor++;
                }
                
                if (status[posX][posY - 1][posZ] == FLUID)
                {
                    neighbor++;
                    //mat.insert(count,layer[i][j-1][k]) = -1.f;
                    tripletList.push_back(T(count,layer[i][j-1][k],-1.0));
                }
                else if (status[posX][posY - 1][posZ] == AIR)
                {
                    neighbor++;
                }
                
                if (status[posX][posY + 1][posZ] == FLUID)
                {
                    neighbor++;
                    //mat.insert(count,layer[i][j+1][k]) = -1.f;
                    tripletList.push_back(T(count,layer[i][j+1][k],-1.0));
                }
                else if (status[posX][posY + 1][posZ] == AIR)
                {
                    neighbor++;
                }
                
                if (status[posX][posY][posZ - 1] == FLUID)
                {
                    neighbor++;
                    //mat.insert(count,layer[i][j][k-1]) = -1.f;
                    tripletList.push_back(T(count,layer[i][j][k-1],-1.0));
                }
                else if (status[posX][posY][posZ - 1] == AIR)
                {
                    neighbor++;
                }
                
                if (status[posX][posY][posZ + 1] == FLUID)
                {
                    neighbor++;
                    //mat.insert(count,layer[i][j][k+1]) = -1.f;
                    tripletList.push_back(T(count,layer[i][j][k+1],-1.0));
                }
                else if (status[posX][posY][posZ + 1] == AIR)
                {
                    neighbor++;
                }
                
                //mat.insert(count,count) = (float)(neighbor);
                tripletList.push_back(T(count,count,(double)(neighbor)));
                count++;
                
            }
    //std::cout<<b<<endl;
    //std::cout<<mat;
    Eigen::SparseMatrix<double> mat(count, count);
    mat.setFromTriplets(tripletList.begin(), tripletList.end());
    // Solving:
    Eigen::ConjugateGradient<Eigen::SparseMatrix<double> > solver(mat);
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
    
	
    
	maxVelocity = 1.0f;
	
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
    float a =1.f;
    float u0 = 4.f;
    
    for (int i=0;i<Cell_NUM_X+BUFFER*2;i++)
        for (int j=0; j<Cell_NUM_Y+BUFFER*2; j++){
            w[i][j][1] = a*w[i][j][1];
            w[i][j][0] = a*w[i][j][1];
            u[i][j][0] = a*u[i][j][1];
            v[i][j][0] = a*v[i][j][1];
            w[i][j][Cell_NUM_Z+BUFFER*2-1] = a*w[i][j][Cell_NUM_Z+BUFFER*2-2];
            u[i][j][Cell_NUM_Z+BUFFER*2-1] = a*u[i][j][Cell_NUM_Z+BUFFER*2-2];
            v[i][j][Cell_NUM_Z+BUFFER*2-1] = a*v[i][j][Cell_NUM_Z+BUFFER*2-2];
            
        }
    for (int i=0;i<Cell_NUM_X+BUFFER*2;i++)
        for (int k=0; k<Cell_NUM_Y+BUFFER*2; k++){
            v[i][1][k] = a*v[i][1][k];
            u[i][0][k] = a*u[i][1][k];
            v[i][0][k] = a*v[i][1][k];
            w[i][0][k] = a*w[i][1][k];
            u[i][Cell_NUM_Y+BUFFER*2-1][k] = a*u[i][Cell_NUM_Y+BUFFER*2-2][k];
            v[i][Cell_NUM_Y+BUFFER*2-1][k] = a*v[i][Cell_NUM_Y+BUFFER*2-2][k];
            w[i][Cell_NUM_Y+BUFFER*2-1][k] = a*w[i][Cell_NUM_Y+BUFFER*2-2][k];
            
        }
    for (int j=0; j<Cell_NUM_Y+BUFFER*2; j++)
        for (int k=0; k<Cell_NUM_Y+BUFFER*2; k++){
            
                u[1][j][k] = u0;
                u[0][j][k] = u0;
           
            v[0][j][k] = a*v[1][j][k];
            w[0][j][k] = a*w[1][j][k];
            u[Cell_NUM_X+BUFFER*2-1][j][k] =  u0;//a*u[Cell_NUM_X +BUFFER*2 -2][j][k];
            v[Cell_NUM_X+BUFFER*2-1][j][k] = a*v[Cell_NUM_X +BUFFER*2 -2][j][k];
            w[Cell_NUM_X+BUFFER*2-1][j][k] = a*w[Cell_NUM_X +BUFFER*2 -2][j][k];
            
        }
    
}



//move particles for time t
void Fluid::MoveParticles(float time)
{
	for (list<struct Particle *>::iterator iter = listParticles.begin(); iter != listParticles.end();iter++)
	{
		Vector3f v = getVelocity((*iter)->pos[0],(*iter)->pos[1], (*iter)->pos[2]);
		
        if ((*iter)->inBound){
            (*iter)->vel = v;
        }
        
        (*iter)->pos += (*iter)->vel * time;
	}
}

void Fluid::AddSource(){
    float y,z;
    for (int j = 1; j < Cell_NUM_Y-3; j++)
        for (int k = 1; k < Cell_NUM_Z-3; k++)
        {
            for(int step = 0; step<1; step++){
                y = j+((float) rand()) / (float) RAND_MAX;
                z = k+((float) rand()) / (float) RAND_MAX;
                
                struct Particle* newp = new struct Particle(Vector3f(0. , y* CELL_WIDTH, z * CELL_WIDTH), Vector3f(0,0,0));
                listParticles.push_back(newp);
            }
            
        }
}

float* Fluid::GenVertexArrayInBound(int& inBoundCount){
    float* vertices = new float[3*listParticles.size()];
    int bufferIndex = 0;
    for(list<struct Particle*>::iterator iter=listParticles.begin();iter != listParticles.end();iter++){
        if((*iter)->inBound){
            vertices[bufferIndex++] =(*iter)->pos[0];
            vertices[bufferIndex++] =(*iter)->pos[1];
            vertices[bufferIndex++] =(*iter)->pos[2];
            inBoundCount++;
        }
    }
    return vertices;
}
float* Fluid::GenVertexArrayOutBound(int& outBoundCount){
    float* vertices = new float[3*listParticles.size()];
    int bufferIndex = 0;
    for(list<struct Particle*>::iterator iter=listParticles.begin();iter != listParticles.end();iter++){
        if(!(*iter)->inBound) {
            vertices[bufferIndex++] =(*iter)->pos[0];
            vertices[bufferIndex++] =(*iter)->pos[1];
            vertices[bufferIndex++] =(*iter)->pos[2];
            outBoundCount++;
        }
    }
    return vertices;
}

void Fluid::Rotate(float rx, float ry, float rz){
    Matrix4f rotx, roty, rotz;
    rotx = Matrix4f::Identity();
    roty = Matrix4f::Identity();
    rotz = Matrix4f::Identity();
    float cosrx, sinrx, cosry, sinry, cosrz, sinrz;
    cosrx = cosf(rx); sinrx = sinf(rx);
    cosry = cosf(ry); sinry = sinf(ry);
    cosrz = cosf(rz); sinrz = sinf(rz);
    
    rotx(1,1) = cosrx; rotx(1,2) = -sinrx;
    rotx(2,1) = sinrx; rotx(2,2) = cosrx;
    
    roty(0,0) = cosry; roty(2,0) = -sinry;
    roty(0,2) = sinry; roty(2,2) = cosry;
    
    rotz(0,0) = cosrz; rotz(0,1) = -sinrz;
    rotz(1,0) = sinrz; rotz(1,1) = cosrz;
    
    rot = (rotx * roty * rotz);
}

// Overrides RenderObject::Render
void Fluid::Render(float rx, float ry, float rz) {
    
    glBindFramebuffer(GL_FRAMEBUFFER, pipeline->frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pipeline->colorTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pipeline->depthBuffer);
    glViewport(0, 0, displayWidth, displayHeight);

    // In bound
    int inBoundCount = 0;
    float * mesh = GenVertexArrayInBound(inBoundCount);
    RenderObject::Render(0, mesh, inBoundCount);
    //delete[] mesh;

    // Out bound
    int outBoundCount = 0;
    mesh = GenVertexArrayOutBound(outBoundCount);
    RenderObject::Render(0, mesh, outBoundCount);
    delete[] mesh;
}

// Overrides RenderObject::RenderPass
void Fluid::RenderPass(int instance, GLfloat *buffer, int num) {

    glDisable(GL_DEPTH_TEST); // TODO

    // Pass matrices
    GLfloat* mv_Matrix = (GLfloat*)mvMatrix();
    GLfloat* mvp_Matrix = (GLfloat*)mvpMatrix();
    glUniformMatrix4fv(gmvMatrixHandle, 1, GL_FALSE, mv_Matrix);
    glUniformMatrix4fv(gmvpMatrixHandle, 1, GL_FALSE, mvp_Matrix);
    checkGlError("glUniformMatrix4fv");
    delete[] mv_Matrix;
    delete[] mvp_Matrix;
    
    // Don't use vertex buffering
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    // Pass vertices
    glEnableVertexAttribArray(gvPositionHandle);
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (const GLvoid*)(0 + buffer));
    checkGlError("gvPositionHandle");
    
    glDrawArrays(GL_POINTS, 0, num);
    checkGlError("glDrawArrays");
}


