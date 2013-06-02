#pragma once
#include "Cell.h"
#include "PCG.h"
#include <cmath>
#include <vector>
#include <list>
using std::list;
using std::vector;
using Eigen::Vector3f;


#define Cell_NUM_X 10
#define Cell_NUM_Y 5
#define Cell_NUM_Z 5
#define KCFL 0.9f
#define BUFFER 2
#define DIRECTION_X 0
#define DIRECTION_Y 1
#define DIRECTION_Z 2
#define GRAVITY 100.0f
#define FRAME_TIME 0.04f

class Fluid
{
public:
	Cell grid[Cell_NUM_X+2*BUFFER][Cell_NUM_Y+2*BUFFER][Cell_NUM_Z+2*BUFFER];
	list<Cell *> listCells;
	list<Vector3f*> listParticles;
	float maxVelocity;
	float deltaTime;
	float remainderTime;

	void CreateBuffer(Cell *N, int i);
	bool isInBound(Cell *pCell);
	Cell *getCell(Vector3f &pos);
	float divVelocity(Cell *g);
	Vector3f gradPressure(Cell *C);
	Vector3f traceParticle(float x, float y, float z, float t);
	Vector3f getVelocity(float x, float y, float z);
	float getInterpolatedValue(float x, float y, float z, int direction);

    Fluid();
	void Update();
	void UpdateDeltaTime();
	void UpdateSolid();
	void UpdateCells();
	void ApplyConvection();
	void ApplyGravity();
	void ApplyPressure();
	void UpdateBufferVelocity();
	void SetSolidCells();
	void MoveParticles(float time);
    
    //mobile
    RenderObject* renderer;
    void RenderSetup();
    void RenderFrame();
    float* GenVertexArray();
    float* Surface(TRIANGLE*&, int&);
};

Fluid::Fluid(){
    renderer = new RenderObject("standard_v.glsl", "normals_f.glsl");
}

//is in the simulation bound
bool Fluid::isInBound(Cell *pCell)
{
	if (pCell->x >= 0 && pCell->x < Cell_NUM_X
		&& pCell->y >= 0 && pCell->y < Cell_NUM_Y
		&& pCell->z >= 0 && pCell->z < Cell_NUM_Z)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//set the cell to be buffer cell
void Fluid::CreateBuffer(Cell *N, int i)
{
	if (N->status != NOTHING)
	{
		if (N->layer == -1 && N->status != SOLID)
		{
			N->status = AIR;
			N->layer = i;
		}
		//In the paper this part is missing
		else if (N->layer == -1 && N->status == SOLID)
		{
			N->layer = i;
		}
		//////////////////////////////////////////////
	}
	else
	{
		N->layer = i;
		listCells.push_back(N);
		if (isInBound(N))
		{
			N->status = AIR;
		}
		else
		{
			N->status = SOLID;
		}
	}
}

//get the cell at a certain position
Cell *Fluid::getCell(Vector3f &pos)
{
	int i = static_cast<int>(max((int)floor(pos[0] / CELL_WIDTH), 0));
	int j = static_cast<int>(max((int)floor(pos[1] / CELL_WIDTH), 0));
	int k = static_cast<int>(max((int)floor(pos[2] / CELL_WIDTH), 0));
	grid[BUFFER + i][BUFFER + j][BUFFER + k].x = i;
	grid[BUFFER + i][BUFFER + j][BUFFER + k].y = j;
	grid[BUFFER + i][BUFFER + j][BUFFER + k].z = k;
	return &grid[BUFFER + i][BUFFER + j][BUFFER + k];
}

//trace a particle at a point(x,y,z) for t time
Vector3f Fluid::traceParticle(float x, float y, float z, float t)
{
	Vector3f v = getVelocity(x, y, z);
	v = getVelocity(x + 0.5f * t * v[0], y + 0.5f * t * v[1], z + 0.5f * t * v[2]);
	return Vector3f(x, y, z) + v * t;
}

//get the velocity at position(x,y,z)
Vector3f Fluid::getVelocity(float x, float y, float z)
{
	Vector3f v;
	v[0] = getInterpolatedValue(x / CELL_WIDTH, y / CELL_WIDTH - 0.5f, z / CELL_WIDTH - 0.5f, DIRECTION_X);
	v[1] = getInterpolatedValue(x / CELL_WIDTH - 0.5f, y / CELL_WIDTH, z / CELL_WIDTH - 0.5f, DIRECTION_Y);
	v[2] = getInterpolatedValue(x / CELL_WIDTH - 0.5f, y / CELL_WIDTH - 0.5f, z / CELL_WIDTH, DIRECTION_Z);
	return v;
}

//get an interpolated value from the grid
float Fluid::getInterpolatedValue(float x, float y, float z, int direction)
{
	int i = static_cast<int>(max((int)floor(x), 0));
	int j = static_cast<int>(max((int)floor(y), 0));
	int k = static_cast<int>(max((int)floor(z), 0));
	float weight = 0.0f;
	float sum = 0.0f;
	switch(direction)
	{
	case DIRECTION_X:
		if (grid[BUFFER + i][BUFFER + j][BUFFER + k].status != NOTHING)
		{
			sum += (i + 1 - x) * (j + 1 - y) * (k + 1 - z) * grid[BUFFER + i][BUFFER + j][BUFFER + k].velocity[0];
			weight += (i + 1 - x) * (j + 1 - y) * (k + 1 - z);
		}
		if (grid[BUFFER + i + 1][BUFFER + j][BUFFER + k].status != NOTHING)
		{
			sum += (x - i) * (j + 1 - y) * (k + 1 - z) * grid[BUFFER + i + 1][BUFFER + j][BUFFER + k].velocity[0];
			weight += (x - i) * (j + 1 - y) * (k + 1 - z);
		}
		if (grid[BUFFER + i][BUFFER + j + 1][BUFFER + k].status != NOTHING)
		{
			sum += (i + 1 - x) * (y - j) * (k + 1 - z) * grid[BUFFER + i][BUFFER + j + 1][BUFFER + k].velocity[0];
			weight += (i + 1 - x) * (y - j) * (k + 1 - z);
		}
		if (grid[BUFFER + i + 1][BUFFER + j + 1][BUFFER + k].status != NOTHING)
		{
			sum += (x - i) * (y - j) * (k + 1 - z) * grid[BUFFER + i + 1][BUFFER + j + 1][BUFFER + k].velocity[0];
			weight += (x - i) * (y - j) * (k + 1 - z);
		}
		if (grid[BUFFER + i][BUFFER + j][BUFFER + k + 1].status != NOTHING)
		{
			sum += (i + 1 - x) * (j + 1 - y) * (z - k) * grid[BUFFER + i][BUFFER + j][BUFFER + k + 1].velocity[0];
			weight += (i + 1 - x) * (j + 1 - y) * (z - k);
		}
		if (grid[BUFFER + i + 1][BUFFER + j][BUFFER + k + 1].status != NOTHING)
		{
			sum += (x - i) * (j + 1 - y) * (z - k) * grid[BUFFER + i + 1][BUFFER + j][BUFFER + k + 1].velocity[0];
			weight += (x - i) * (j + 1 - y) * (z - k);
		}
		if (grid[BUFFER + i][BUFFER + j + 1][BUFFER + k + 1].status != NOTHING)
		{
			sum += (i + 1 - x) * (y - j) * (z - k) * grid[BUFFER + i][BUFFER + j + 1][BUFFER + k + 1].velocity[0];
			weight += (i + 1 - x) * (y - j) * (z - k);
		}
		if (grid[BUFFER + i + 1][BUFFER + j + 1][BUFFER + k + 1].status != NOTHING)
		{
			sum += (x - i) * (y - j) * (z - k) * grid[BUFFER + i + 1][BUFFER + j + 1][BUFFER + k + 1].velocity[0];
			weight += (x - i) * (y - j) * (z - k);
		}
		return sum / weight;
		break;
	case DIRECTION_Y:
		if (grid[BUFFER + i][BUFFER + j][BUFFER + k].status != NOTHING)
		{
			sum += (i + 1 - x) * (j + 1 - y) * (k + 1 - z) * grid[BUFFER + i][BUFFER + j][BUFFER + k].velocity[1];
			weight += (i + 1 - x) * (j + 1 - y) * (k + 1 - z);
		}
		if (grid[BUFFER + i + 1][BUFFER + j][BUFFER + k].status != NOTHING)
		{
			sum += (x - i) * (j + 1 - y) * (k + 1 - z) * grid[BUFFER + i + 1][BUFFER + j][BUFFER + k].velocity[1];
			weight += (x - i) * (j + 1 - y) * (k + 1 - z);
		}
		if (grid[BUFFER + i][BUFFER + j + 1][BUFFER + k].status != NOTHING)
		{
			sum += (i + 1 - x) * (y - j) * (k + 1 - z) * grid[BUFFER + i][BUFFER + j + 1][BUFFER + k].velocity[1];
			weight += (i + 1 - x) * (y - j) * (k + 1 - z);
		}
		if (grid[BUFFER + i + 1][BUFFER + j + 1][BUFFER + k].status != NOTHING)
		{
			sum += (x - i) * (y - j) * (k + 1 - z) * grid[BUFFER + i + 1][BUFFER + j + 1][BUFFER + k].velocity[1];
			weight += (x - i) * (y - j) * (k + 1 - z);
		}
		if (grid[BUFFER + i][BUFFER + j][BUFFER + k + 1].status != NOTHING)
		{
			sum += (i + 1 - x) * (j + 1 - y) * (z - k) * grid[BUFFER + i][BUFFER + j][BUFFER + k + 1].velocity[1];
			weight += (i + 1 - x) * (j + 1 - y) * (z - k);
		}
		if (grid[BUFFER + i + 1][BUFFER + j][BUFFER + k + 1].status != NOTHING)
		{
			sum += (x - i) * (j + 1 - y) * (z - k) * grid[BUFFER + i + 1][BUFFER + j][BUFFER + k + 1].velocity[1];
			weight += (x - i) * (j + 1 - y) * (z - k);
		}
		if (grid[BUFFER + i][BUFFER + j + 1][BUFFER + k + 1].status != NOTHING)
		{
			sum += (i + 1 - x) * (y - j) * (z - k) * grid[BUFFER + i][BUFFER + j + 1][BUFFER + k + 1].velocity[1];
			weight += (i + 1 - x) * (y - j) * (z - k);
		}
		if (grid[BUFFER + i + 1][BUFFER + j + 1][BUFFER + k + 1].status != NOTHING)
		{
			sum += (x - i) * (y - j) * (z - k) * grid[BUFFER + i + 1][BUFFER + j + 1][BUFFER + k + 1].velocity[1];
			weight += (x - i) * (y - j) * (z - k);
		}
		return sum / weight;
		break;
	case DIRECTION_Z:
		if (grid[BUFFER + i][BUFFER + j][BUFFER + k].status != NOTHING)
		{
			sum += (i + 1 - x) * (j + 1 - y) * (k + 1 - z) * grid[BUFFER + i][BUFFER + j][BUFFER + k].velocity[2];
			weight += (i + 1 - x) * (j + 1 - y) * (k + 1 - z);
		}
		if (grid[BUFFER + i + 1][BUFFER + j][BUFFER + k].status != NOTHING)
		{
			sum += (x - i) * (j + 1 - y) * (k + 1 - z) * grid[BUFFER + i + 1][BUFFER + j][BUFFER + k].velocity[2];
			weight += (x - i) * (j + 1 - y) * (k + 1 - z);
		}
		if (grid[BUFFER + i][BUFFER + j + 1][BUFFER + k].status != NOTHING)
		{
			sum += (i + 1 - x) * (y - j) * (k + 1 - z) * grid[BUFFER + i][BUFFER + j + 1][BUFFER + k].velocity[2];
			weight += (i + 1 - x) * (y - j) * (k + 1 - z);
		}
		if (grid[BUFFER + i + 1][BUFFER + j + 1][BUFFER + k].status != NOTHING)
		{
			sum += (x - i) * (y - j) * (k + 1 - z) * grid[BUFFER + i + 1][BUFFER + j + 1][BUFFER + k].velocity[2];
			weight += (x - i) * (y - j) * (k + 1 - z);
		}
		if (grid[BUFFER + i][BUFFER + j][BUFFER + k + 1].status != NOTHING)
		{
			sum += (i + 1 - x) * (j + 1 - y) * (z - k) * grid[BUFFER + i][BUFFER + j][BUFFER + k + 1].velocity[2];
			weight += (i + 1 - x) * (j + 1 - y) * (z - k);
		}
		if (grid[BUFFER + i + 1][BUFFER + j][BUFFER + k + 1].status != NOTHING)
		{
			sum += (x - i) * (j + 1 - y) * (z - k) * grid[BUFFER + i + 1][BUFFER + j][BUFFER + k + 1].velocity[2];
			weight += (x - i) * (j + 1 - y) * (z - k);
		}
		if (grid[BUFFER + i][BUFFER + j + 1][BUFFER + k + 1].status != NOTHING)
		{
			sum += (i + 1 - x) * (y - j) * (z - k) * grid[BUFFER + i][BUFFER + j + 1][BUFFER + k + 1].velocity[2];
			weight += (i + 1 - x) * (y - j) * (z - k);
		}
		if (grid[BUFFER + i + 1][BUFFER + j + 1][BUFFER + k + 1].status != NOTHING)
		{
			sum += (x - i) * (y - j) * (z - k) * grid[BUFFER + i + 1][BUFFER + j + 1][BUFFER + k + 1].velocity[2];
			weight += (x - i) * (y - j) * (z - k);
		}
		return sum / weight;
		break;
	default:
		return 0;
	}
}

//calculate the divergence of velocity at the centre of a cell
float Fluid::divVelocity(Cell *g) 
{
	int posX = BUFFER + g->x;
	int posY = BUFFER + g->y;
	int posZ = BUFFER + g->z;
	float ret = 0.0f;
	if (grid[posX - 1][posY][posZ].status != SOLID)
	{
		ret += - g->velocity[0];
	}

	if (grid[posX][posY - 1][posZ].status != SOLID)
	{
		ret += - g->velocity[1];
	}

	if (grid[posX][posY][posZ - 1].status != SOLID)
	{
		ret += - g->velocity[2];
	}

	if (grid[posX+1][posY][posZ].status != SOLID)
	{
		ret += grid[posX+1][posY][posZ].velocity[0];
	}
	

	if (grid[posX][posY+1][posZ].status != SOLID)
	{
		ret += grid[posX][posY+1][posZ].velocity[1];
	}
	

	if (grid[posX][posY][posZ+1].status != SOLID)
	{
		ret += grid[posX][posY][posZ+1].velocity[2];
	}
	
	
	return ret;
}

//calculate the gradient of pressure at the center of the cell
Vector3f Fluid::gradPressure(Cell *C)
{
	Vector3f v(0.0f, 0.0f, 0.0f);
	int posX = BUFFER + C->x;
	int posY = BUFFER + C->y;
	int posZ = BUFFER + C->z;
	if (grid[posX - 1][posY][posZ].status == AIR)
	{
		v[0] = C->pressure;
	}
	else if (grid[posX - 1][posY][posZ].status == FLUID)
	{
		v[0] = C->pressure - grid[posX - 1][posY][posZ].pressure;
	}
	if (grid[posX][posY - 1][posZ].status == AIR)
	{
		v[1] = C->pressure;
	}
	else if (grid[posX][posY - 1][posZ].status == FLUID)
	{
		v[1] = C->pressure - grid[posX][posY - 1][posZ].pressure;
	}
	if (grid[posX][posY][posZ - 1].status == AIR)
	{
		v[2] = C->pressure;
	}
	else if (grid[posX][posY][posZ - 1].status == FLUID)
	{
		v[2] = C->pressure - grid[posX][posY][posZ - 1].pressure;
	}

	return v;
}

//update the status of fluid
void Fluid::Update()
{
	if (remainderTime >= FRAME_TIME)
	{
		remainderTime -= FRAME_TIME;
		MoveParticles(FRAME_TIME);
	}
	else
	{
		while (remainderTime < FRAME_TIME)
		{
			UpdateDeltaTime();		//1
			MoveParticles(deltaTime);
			//UpdateSolid();
			UpdateCells();			//2
			ApplyConvection();		//3a
			ApplyGravity();			//3b
			ApplyPressure();		//3de
			UpdateBufferVelocity();	//3f
			SetSolidCells();		//3g
			remainderTime += deltaTime;
		}
		remainderTime -= FRAME_TIME;
		MoveParticles(- remainderTime + deltaTime);
	}
}

//calculate the simulation time step
void Fluid::UpdateDeltaTime()
{
	deltaTime = KCFL * CELL_WIDTH / maxVelocity;
	if (deltaTime > FRAME_TIME)
	{
		deltaTime = FRAME_TIME;
	}
}

//calculate the new status of solid
void Fluid::UpdateSolid()
{
	for (list<Cell *>::iterator iter = listCells.begin(); iter != listCells.end(); iter++)
	{
		Cell *C = *iter;
		if (C->status == SOLID)
		{
			if (isInBound(C) == true)
			{
				C->status = AIR;
			}
		}
	}
}

//update the grid based on the marker particles
void Fluid::UpdateCells()
{
	for (list<Cell *>::iterator iter = listCells.begin(); iter != listCells.end(); iter++)
	{		
		(*iter)->layer = -1;
	}
	for (list<Vector3f*>::iterator iter = listParticles.begin(); iter != listParticles.end();)
	{
        Cell *pCell = getCell(**iter);
		if (pCell->status == NOTHING)
		{
			if (isInBound(pCell))
			{
				pCell->status = FLUID;
				pCell->layer = 0;
				listCells.push_back(pCell);
			}
		}
		else if (pCell->status != SOLID)	
		{
			pCell->status = FLUID;
			pCell->layer = 0;
		}
		else
		{
			delete(*iter);
			iter = listParticles.erase(iter);
			continue;
		}
		iter++;
	}
	for (int i = 1; i <= 2 || i <= BUFFER; i++)
	{
		for (list<Cell *>::iterator iter = listCells.begin(); iter != listCells.end(); iter++)
		{
			if ((*iter)->status != SOLID && (*iter)->status != NOTHING && (*iter)->layer == i - 1)
			{
				Cell *C = *iter;
				Cell *N;
				int posx = BUFFER + C->x;
				int posy = BUFFER + C->y;
				int posz = BUFFER + C->z;

				N = &grid[posx - 1][posy][posz];
				N->x = C->x - 1;
				N->y = C->y;
				N->z = C->z;
				CreateBuffer(N, i);

				N = &grid[posx + 1][posy][posz];
				N->x = C->x + 1;
				N->y = C->y;
				N->z = C->z;
				CreateBuffer(N, i);

				N = &grid[posx][posy - 1][posz];
				N->x = C->x;
				N->y = C->y - 1;
				N->z = C->z;
				CreateBuffer(N, i);

				N = &grid[posx][posy + 1][posz];
				N->x = C->x;
				N->y = C->y + 1;
				N->z = C->z;
				CreateBuffer(N, i);

				N = &grid[posx][posy][posz - 1];
				N->x = C->x;
				N->y = C->y;
				N->z = C->z - 1;
				CreateBuffer(N, i);

				N = &grid[posx][posy][posz + 1];
				N->x = C->x;
				N->y = C->y;
				N->z = C->z + 1;
				CreateBuffer(N, i);
			}
		}
	}
	for (list<Cell *>::iterator iter = listCells.begin(); iter != listCells.end();)
	{
		if ((*iter)->layer == -1)
		{
			Cell *c = *iter;
			memset(c, 0, sizeof(Cell));
			iter = listCells.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

//apply convection using a backwards particle trace
void Fluid::ApplyConvection()
{
	for (list<Cell *>::iterator iter = listCells.begin(); iter != listCells.end(); iter++)
	{
		Cell *g = *iter;
		int posX = BUFFER + g->x;
		int posY = BUFFER + g->y;
		int posZ = BUFFER + g->z;
		if (g->status == FLUID)
		{
			Vector3f v;
			v = this->traceParticle(g->x * CELL_WIDTH, g->y * CELL_WIDTH + 0.5f, g->z * CELL_WIDTH + 0.5f, deltaTime);
			g->newvelocity[0] = this->getVelocity(v[0], v[1], v[2])[0];
			v = this->traceParticle(g->x * CELL_WIDTH + 0.5f, g->y * CELL_WIDTH, g->z * CELL_WIDTH + 0.5f, deltaTime);
			g->newvelocity[1] = this->getVelocity(v[0], v[1], v[2])[1];
			v = this->traceParticle(g->x * CELL_WIDTH + 0.5f, g->y * CELL_WIDTH + 0.5f, g->z * CELL_WIDTH, deltaTime);
			g->newvelocity[2] = this->getVelocity(v[0], v[1], v[2])[2];
		}
		else if (grid[posX-1][posY][posZ].status == FLUID)
		{
			Vector3f v;
			g->newvelocity = g->velocity;
			v = this->traceParticle(g->x * CELL_WIDTH, g->y * CELL_WIDTH + 0.5f, g->z * CELL_WIDTH + 0.5f, deltaTime);
			g->newvelocity[0] = this->getVelocity(v[0], v[1], v[2])[0];
		}
		else if (grid[posX][posY-1][posZ].status == FLUID)
		{
			Vector3f v;
			g->newvelocity = g->velocity;
			v = this->traceParticle(g->x * CELL_WIDTH + 0.5f, g->y * CELL_WIDTH, g->z * CELL_WIDTH + 0.5f, deltaTime);
			g->newvelocity[1] = this->getVelocity(v[0], v[1], v[2])[1];
		}
		else if (grid[posX][posY][posZ-1].status == FLUID)
		{
			Vector3f v;
			g->newvelocity = g->velocity;
			v = this->traceParticle(g->x * CELL_WIDTH + 0.5f, g->y * CELL_WIDTH + 0.5f, g->z * CELL_WIDTH, deltaTime);
			g->newvelocity[2] = this->getVelocity(v[0], v[1], v[2])[2];
		}
		else
		{
			g->newvelocity = g->velocity;
		}
	}
	maxVelocity = 0.0f;
	for (list<Cell *>::iterator iter = listCells.begin(); iter != listCells.end(); iter++)
	{
		Cell *C = *iter;
		float l = C->newvelocity.norm();
		if (maxVelocity < l)
		{
			maxVelocity = l;
		}
		C->velocity = C->newvelocity;
	}
}

//apply gravity(external force)
void Fluid::ApplyGravity()
{
	maxVelocity = 0.0f;
	for (list<Cell *>::iterator iter = listCells.begin(); iter != listCells.end(); iter++)
	{
		Cell *g = *iter;
		int posX = BUFFER + g->x;
		int posY = BUFFER + g->y;
		int posZ = BUFFER + g->z;
		if (g->status == FLUID || grid[posX][posY-1][posZ].status == FLUID)
		{
			g->velocity[1] += deltaTime * GRAVITY;
			float l = g->velocity.norm();
			if (l > maxVelocity)
			{
				maxVelocity = l;
			}
		}
	}
}

//apply pressure
void Fluid::ApplyPressure()
{
	vector<Cell *> fluidCells;
	int count = 0;
	for (list<Cell *>::iterator iter = listCells.begin(); iter != listCells.end(); iter++)
	{
		Cell *C = *iter;
		if (C->status == FLUID)
		{
			fluidCells.push_back(C);
			C->layer = count;
			count++;
		}
	}
	float *sra = new float[7 * count];
	int*clm = new int[7 * count];
	int *fnz = new int[count + 1];
	int pos = 0;
	count = 0;
	for (vector<Cell *>::iterator iter = fluidCells.begin(); iter != fluidCells.end(); iter++)
	{
		Cell *C = *iter;
		fnz[count] = pos;
		int neighbor = 0;

		int posX = BUFFER + C->x;
		int posY = BUFFER + C->y;
		int posZ = BUFFER + C->z;
		if (grid[posX - 1][posY][posZ].status == FLUID)
		{
			neighbor++;
			sra[pos] = 1.0f;
			clm[pos] = grid[posX - 1][posY][posZ].layer;
			pos++;
		}
		else if (grid[posX - 1][posY][posZ].status == AIR)
		{
			neighbor++;
		}

		if (grid[posX + 1][posY][posZ].status == FLUID)
		{
			neighbor++;
			sra[pos] = 1.0f;
			clm[pos] = grid[posX + 1][posY][posZ].layer;
			pos++;
		}
		else if (grid[posX + 1][posY][posZ].status == AIR)
		{
			neighbor++;
		}

		if (grid[posX][posY - 1][posZ].status == FLUID)
		{
			neighbor++;
			sra[pos] = 1.0f;
			clm[pos] = grid[posX][posY - 1][posZ].layer;
			pos++;
		}
		else if (grid[posX][posY - 1][posZ].status == AIR)
		{
			neighbor++;
		}

		if (grid[posX][posY + 1][posZ].status == FLUID)
		{
			neighbor++;
			sra[pos] = 1.0f;
			clm[pos] = grid[posX][posY + 1][posZ].layer;
			pos++;
		}
		else if (grid[posX][posY + 1][posZ].status == AIR)
		{
			neighbor++;
		}

		if (grid[posX][posY][posZ - 1].status == FLUID)
		{
			neighbor++;
			sra[pos] = 1.0f;
			clm[pos] = grid[posX][posY][posZ - 1].layer;
			pos++;
		}
		else if (grid[posX][posY][posZ - 1].status == AIR)
		{
			neighbor++;
		}

		if (grid[posX][posY][posZ + 1].status == FLUID)
		{
			neighbor++;
			sra[pos] = 1.0f;
			clm[pos] = grid[posX][posY][posZ + 1].layer;
			pos++;
		}
		else if (grid[posX][posY][posZ + 1].status == AIR)
		{
			neighbor++;
		}
		
		sra[pos] = static_cast<float>(-neighbor);
		clm[pos] = count;
		pos++;
		count++;
	}
	fnz[count] = pos;
	float *b = new float[count];
	for (int i = 0; i != count; i++)
	{
		b[i] = divVelocity(fluidCells[i]);		
	}
	float *x = new float[count];
	for (int i = 0; i != count; i++)
	{
		x[i] = 0.0f;		
	}
	PCG(count, sra, clm, fnz, x, b);
	for (int i = 0; i != count; i++)
	{
		fluidCells[i]->pressure = x[i];
	}
	delete []sra;
	delete []clm;
	delete []b;
	delete []fnz;
	delete []x;
	maxVelocity = 0.0f;
	for (list<Cell *>::iterator iter = listCells.begin(); iter != listCells.end(); iter++)
	{
		Cell *C = *iter;
		if (C->status == SOLID)
		{
			continue;
		}
		if (C->status == AIR)
		{
			C->pressure = 0;
		}
		C->velocity -= gradPressure(C);
		float l = C->velocity.norm();
		if (maxVelocity < l)
		{
			maxVelocity = l;
		}
	}
}

//extrapolate the fluid velocity to the buffer zone
void Fluid::UpdateBufferVelocity()
{
	list<Cell *> nfluidCells;
	for (list<Cell *>::iterator iter = listCells.begin(); iter != listCells.end(); iter++)
	{
		Cell *C = *iter;
		if (C->status == FLUID)
		{
			C->layer = 0;
		}
		else
		{
			C->layer = -1;
			nfluidCells.push_back(C);
		}
	}
	for (int i = 1; i <= 2 || i <= BUFFER; i++)
	{
		for (list<Cell *>::iterator iter = nfluidCells.begin(); iter != nfluidCells.end();)
		{
			Cell *C = *iter;
			bool fN = false;
			Cell *N;
			int count = 0;
			Vector3f sum(0.0f, 0.0f, 0.0f);
			int posX = BUFFER + C->x;
			int posY = BUFFER + C->y;
			int posZ = BUFFER + C->z;
			N = &grid[posX - 1][posY][posZ];
			if (N->layer == i - 1 && N->status != NOTHING)
			{
				fN = true;
				count++;
				sum += N->velocity;
			}
			N = &grid[posX + 1][posY][posZ];
			if (N->layer == i - 1 && N->status != NOTHING)
			{
				fN = true;
				count++;
				sum += N->velocity;
			}
			N = &grid[posX][posY - 1][posZ];
			if (N->layer == i - 1 && N->status != NOTHING)
			{
				fN = true;
				count++;
				sum += N->velocity;
			}
			N = &grid[posX][posY + 1][posZ];
			if (N->layer == i - 1 && N->status != NOTHING)
			{
				fN = true;
				count++;
				sum += N->velocity;
			}
			N = &grid[posX][posY][posZ - 1];
			if (N->layer == i - 1 && N->status != NOTHING)
			{
				fN = true;
				count++;
				sum += N->velocity;
			}
			N = &grid[posX][posY][posZ + 1];
			if (N->layer == i - 1 && N->status != NOTHING)
			{
				fN = true;
				count++;
				sum += N->velocity;
			}
			if (fN)
			{
				if (grid[posX - 1][posY][posZ].status != FLUID)
				{
					C->velocity[0] = sum[0] / static_cast<float>(count);
				}
				if (grid[posX][posY - 1][posZ].status != FLUID)
				{
					C->velocity[1] = sum[1] / static_cast<float>(count);
				}
				if (grid[posX][posY][posZ - 1].status != FLUID)
				{
					C->velocity[2] = sum[2] / static_cast<float>(count);
				}
				C->layer = i;
				iter = nfluidCells.erase(iter);
			}
			else
			{
				iter++;
			}
		}
	}
    /*
	for (list<Vector *>::iterator iter = listParticles.begin(); iter != listParticles.end(); iter++)
	{
		Particle *p = *iter;
		p->velocity = getVelocity(p->position.x, p->position.y, p->position.z);
	}*/
}

//set solid cell velocities
void Fluid::SetSolidCells()
{
	for (list<Cell *>::iterator iter = listCells.begin(); iter != listCells.end(); iter++)
	{
		Cell *C = *iter;
		int posX = BUFFER + C->x;
		int posY = BUFFER + C->y;
		int posZ = BUFFER + C->z;
		if (C->status == SOLID)
		{
			if (grid[posX - 1][posY][posZ].status != SOLID && C->velocity[0] < 0)
			{
				//C->velocity.x = - C->velocity.x;
				C->velocity[0] = 0;
			}
			if (grid[posX][posY - 1][posZ].status != SOLID && C->velocity[1] < 0)
			{
				//C->velocity.y = - C->velocity.y;
				C->velocity[1] = 0;
			}
			if (grid[posX][posY][posZ - 1].status != SOLID && C->velocity[2] < 0)
			{
				//C->velocity.z = - C->velocity.z;
				C->velocity[2] = 0;
			}
		}
		if (C->status != SOLID)
		{
			if (grid[posX - 1][posY][posZ].status == SOLID && C->velocity[0] > 0)
			{
				//C->velocity.x = - C->velocity.x;
				C->velocity[0] = 0;
			}
			if (grid[posX][posY - 1][posZ].status == SOLID && C->velocity[1] > 0)
			{
				//C->velocity.y = - C->velocity.y;
				C->velocity[1] = 0;
			}
			if (grid[posX][posY][posZ - 1].status == SOLID && C->velocity[2] > 0)
			{
				//C->velocity.z = - C->velocity.z;
				C->velocity[2] = 0;
			}
		}
	}
}

//move particles for time t
void Fluid::MoveParticles(float time)
{
	for (list<Vector3f *>::iterator iter = listParticles.begin(); iter != listParticles.end();)
	{
		Vector3f v = getVelocity((**iter)[0],(**iter)[1], (**iter)[2]);
		//p->velocity = getVelocity(p->position.x, p->position.y, p->position.z) * -1.0f;
		//p->position -= v * time;
		/*if (!isInBound(getCell(p->position)))
		{
			delete (*iter);
			iter = listParticles.erase(iter);
		}
		else
		{
			iter++;
		}*/


		/*if (isInBound(getCell(p->position - v * time)))
		{
			p->position -= v * time;
		}*/
        Vector3f test = **iter - v*time;
		if (getCell(test)->status != SOLID)
		{
			**iter -= v * time;
		}
        test = **iter - v*time;
		if (getCell(test)->z > 90)
		{
			delete (*iter);
			iter = listParticles.erase(iter);
			continue;
		}

		iter++;
	}
}

float* Fluid::GenVertexArray(){
    float* vertices = new float[6*8*listParticles.size()];
    int bufferIndex = 0;
    float radius = .1;
    Vector3f center(0,0,0);
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
        
        center += **iter;
    }
    center /= listParticles.size();
    printf("%f %f %f\n", center[0],center[1],center[2]);
    return vertices;
}

void Fluid::RenderFrame(){
    Update();
    TRIANGLE *tri = NULL;
	int ntri = 0;
    float * mesh = Surface(tri,ntri);
    free(tri);
    renderer->RenderFrame(mesh, ntri);
    delete[] mesh;
}


float* Fluid::Surface(TRIANGLE*& tri, int& ntri){
    //vertex grid store signed distance
    int R=1;
    mcCell mcgrid[BUFFER*2+R*Cell_NUM_X][BUFFER*2+R*Cell_NUM_Y][BUFFER*2+R*Cell_NUM_Z];
    for (int i=0;i<Cell_NUM_X+BUFFER*2;i++) {
		for (int j=0;j<Cell_NUM_Y+BUFFER*2;j++) {
			for (int k=0;k<Cell_NUM_Z+BUFFER*2;k++) {
                //mcgrid[BUFFER+i][BUFFER+j][BUFFER+k].m=100;
                mcgrid[i][j][k].visited = false;
            }
        }
    }
    //GRIDCELL mcgrid;
    //iterate through particles, locally update
    for(list<Vector3f*>::iterator iter=listParticles.begin();iter != listParticles.end();iter++){
        float px = (**iter)[0], py = (**iter)[1], pz = (**iter)[2];
	    int i = static_cast<int>(max((int)floor(px / CELL_WIDTH), 0));
	    int j = static_cast<int>(max((int)floor(py / CELL_WIDTH), 0));
	    int k = static_cast<int>(max((int)floor(pz / CELL_WIDTH), 0));
        
        float x = i*CELL_WIDTH;
        float y = j*CELL_WIDTH;
        float z = k*CELL_WIDTH;
        
        i+=BUFFER;
        j+=BUFFER;
        k+=BUFFER;
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
        d = sqrt((x+CELL_WIDTH-px)*(x+CELL_WIDTH-px)+(y-py)*(y-py)+(z-pz)*(z-pz));
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
        d = sqrt((x-px)*(x-px)+(y+CELL_WIDTH-py)*(y+CELL_WIDTH-py)+(z-pz)*(z-pz));
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
        d = sqrt((x+CELL_WIDTH-px)*(x+CELL_WIDTH-px)+(y+CELL_WIDTH-py)*(y+CELL_WIDTH-py)+(z-pz)*(z-pz));
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
        d = sqrt((x-px)*(x-px)+(y-py)*(y-py)+(z+CELL_WIDTH-pz)*(z+CELL_WIDTH-pz));
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
        d = sqrt((x+CELL_WIDTH-px)*(x+CELL_WIDTH-px)+(y-py)*(y-py)+(z+CELL_WIDTH-pz)*(z+CELL_WIDTH-pz));
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
        d = sqrt((x-px)*(x-px)+(y+CELL_WIDTH-py)*(y+CELL_WIDTH-py)+(z+CELL_WIDTH-pz)*(z+CELL_WIDTH-pz));
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
        d = sqrt((x+CELL_WIDTH-px)*(x+CELL_WIDTH-px)+(y+CELL_WIDTH-py)*(y+CELL_WIDTH-py)+(z+CELL_WIDTH-pz)*(z+CELL_WIDTH-pz));
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
    GRIDCELL Grid;
    TRIANGLE triangles[10];
	
    int n=0;
    float radius=1.f;
    //iterate through all cells and construct triangle mesh
    // Polygonise the grid
	fprintf(stderr,"Polygonising data ...\n");
	for (list<Cell *>::iterator iter = listCells.begin(); iter != listCells.end(); iter++)
	{
		Cell *g = *iter;
		int i = BUFFER + g->x;
		int j = BUFFER + g->y;
		int k = BUFFER + g->z;
        /*if(grid[i-1][j][k].status == FLUID && grid[i+1][j][k].status == FLUID
         && grid[i][j-1][k].status == FLUID && grid[i][j+1][k].status == FLUID
         && grid[i][j][k-1].status == FLUID && grid[i][j][k+1].status == FLUID)
         continue;*/
        if(grid[i+1][j][k].status==FLUID && (grid[i-1][j][k].status!=AIR
                                                && grid[i][j+1][k].status!=AIR && grid[i][j-1][k].status!=AIR
                                                && grid[i][j][k+1].status!=AIR && grid[i][j][k-1].status!=AIR))
            continue;
        float x=g->x*CELL_WIDTH, y= g->y*CELL_WIDTH, z = g->z*CELL_WIDTH;
        //vertex 0: i,j,k
        Grid.p[0].x = x;
        Grid.p[0].y = y;
        Grid.p[0].z = z;
        if(mcgrid[i][j][k].visited)
            Grid.val[0] = mcgrid[i][j][k].min<radius?-mcgrid[i][j][k].max:mcgrid[i][j][k].min;
        else
            Grid.val[0] = CELL_WIDTH;
        //1:i+1,j,k
        Grid.p[1].x = x+CELL_WIDTH;
        Grid.p[1].y = y;
        Grid.p[1].z = z;
        if(mcgrid[i+1][j][k].visited)
            Grid.val[1] = mcgrid[i+1][j][k].min<radius?-mcgrid[i+1][j][k].max:mcgrid[i+1][j][k].min;
        else
            Grid.val[1] = CELL_WIDTH;
        //2:i+1,j+1,k
        Grid.p[2].x = x+CELL_WIDTH;
        Grid.p[2].y = y+CELL_WIDTH;
        Grid.p[2].z = z;
        if(mcgrid[i+1][j+1][k].visited)
            Grid.val[2] = mcgrid[i+1][j+1][k].min<radius?-mcgrid[i+1][j+1][k].max:mcgrid[i+1][j+1][k].min;
        else
            Grid.val[2] = CELL_WIDTH;
        //3:i,j+1,k
        Grid.p[3].x = x;
        Grid.p[3].y = y+CELL_WIDTH;
        Grid.p[3].z = z;
        if(mcgrid[i][j+1][k].visited)
            Grid.val[3] = mcgrid[i][j+1][k].min<radius?-mcgrid[i][j+1][k].max:mcgrid[i][j+1][k].min;
        else
            Grid.val[3] = CELL_WIDTH;
        //4:i,j,k+1
        Grid.p[4].x = x;
        Grid.p[4].y = y;
        Grid.p[4].z = z+CELL_WIDTH;
        if(mcgrid[i][j][k+1].visited)
            Grid.val[4] = mcgrid[i][j][k+1].min<radius?-mcgrid[i][j][k+1].max:mcgrid[i][j][k+1].min;
        else
            Grid.val[4] = CELL_WIDTH;
        //5:i+1,j,k+1
        Grid.p[5].x = x+CELL_WIDTH;
        Grid.p[5].y = y;
        Grid.p[5].z = z+CELL_WIDTH;
        if(mcgrid[i+1][j][k+1].visited)
            Grid.val[5] = mcgrid[i+1][j][k+1].min<radius?-mcgrid[i+1][j][k+1].max:mcgrid[i+1][j][k+1].min;
        else
            Grid.val[5] = CELL_WIDTH;
        //6:i+1,j+1,k+1
        Grid.p[6].x = x+CELL_WIDTH;
        Grid.p[6].y = y+CELL_WIDTH;
        Grid.p[6].z = z+CELL_WIDTH;
        if(mcgrid[i+1][j+1][k+1].visited)
            Grid.val[6] = mcgrid[i+1][j+1][k+1].min<radius?-mcgrid[i+1][j+1][k+1].max:mcgrid[i+1][j+1][k+1].min;
        else
            Grid.val[6] = CELL_WIDTH;
        //7:i,j+1,k+1
        Grid.p[7].x = x;
        Grid.p[7].y = y+CELL_WIDTH;
        Grid.p[7].z = z+CELL_WIDTH;
        if(mcgrid[i][j+1][k+1].visited)
            Grid.val[7] = mcgrid[i][j+1][k+1].min<radius?-mcgrid[i][j+1][k+1].max:mcgrid[i][j+1][k+1].min;
        else
            Grid.val[7] = CELL_WIDTH;
        
        /*for(int s=0;s<8;s++)
         printf("%f ", Grid.val[s]);
         printf("\n");*/
        n = PolygoniseCube(Grid,0.0,triangles);
        tri = (TRIANGLE*)realloc(tri,(ntri+n)*sizeof(TRIANGLE));
        for (int l=0;l<n;l++)
            tri[ntri+l] = triangles[l];
        ntri += n;
	}
    
    float* vertices = new float[3*(3+3)*ntri];
    unsigned int bufferIndex = 0;
    for (int i=0;i<ntri;i++) {
        for (int k=0;k<3;k++)  {
            vertices[bufferIndex++] =tri[i].p[k].x;
            vertices[bufferIndex++] =tri[i].p[k].y;
            vertices[bufferIndex++] =tri[i].p[k].z;
            vertices[bufferIndex++] =tri[i].n[k].x;
            vertices[bufferIndex++] =tri[i].n[k].y;
            vertices[bufferIndex++] =tri[i].n[k].z;
        }
	}
   
    //center /= listParticles.size();
    //printf("%f %f %f\n", center[0],center[1],center[2]);
    return vertices;
    
    //calculate the signed distance value for the cell
}
