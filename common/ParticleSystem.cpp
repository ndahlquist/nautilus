#include "ParticleSystem.h"
#include "Eigen/Sparse"
#include <cmath>
#include <vector>

using std::vector;

typedef Eigen::SparseMatrix<double,Eigen::RowMajor> SpMat;


unsigned int randhash(unsigned int seed)
{
    unsigned int i=(seed^0xA3C59AC3u)*2654435769u;
    i^=(i>>16);
    i*=2654435769u;
    i^=(i>>16);
    i*=2654435769u;
    return i;
}
float randhashf(unsigned int seed, float a, float b)
{ return ( (b-a)*randhash(seed)/(float)UINT_MAX + a); }

Fluid::Fluid(){
	particleRadius=CELL_WIDTH/sqrt(2.0);
	
    memset(u, 0, sizeof(u));
	memset(v, 0, sizeof(v));
	memset(status, 0, sizeof(status));
	memset(newu, 0, sizeof(newu));
	memset(newv, 0, sizeof(newv));
	
    deltaTime=0.0;
    maxVelocity=1.0;
    
    for(int i = 0; i < 10000; ++i) {
        float x = randhashf(i*2, 0,1);
        float y = randhashf(i*2+1, 0,1);
        if(x<=i_high*CELL_WIDTH&&x>=0.50&&y>=j_low*CELL_WIDTH&&y<=(j_high-10)*CELL_WIDTH){
            Vector2f newParticle;
            newParticle(0)=x;
            newParticle(1)=y;
            listParticles.push_back(newParticle);
        }
    }
    
    for(int j=j_low;j<j_high;j++){
        status[i_low-1][j]=SOLID_WALL;
        status[i_high][j]=SOLID_WALL;
    }
    for(int i=i_low-1;i<i_high;i++){
        status[i][j_low-1]=SOLID_WALL;
    }
    renderer = new RenderObject("standard_v.glsl", "diffuse_f.glsl");
    renderer->AddVertex(GenVertexArray(),listParticles.size()*6);
}
float* Fluid::GenVertexArray(){
    float* vertices = new float[6*8*listParticles.size()];
    int bufferIndex = 0;
    float radius = 0.1;
    for(list<Vector2f>::iterator iter=listParticles.begin();iter != listParticles.end();iter++){
        vertices[bufferIndex++] =(*iter)(0)-radius;
        vertices[bufferIndex++] =(*iter)(1)-radius;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =1.f;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =0.f;
        
        vertices[bufferIndex++] =(*iter)(0)+radius;
        vertices[bufferIndex++] =(*iter)(1)-radius;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =1.f;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =0.f;
        
        vertices[bufferIndex++] =(*iter)(0)+radius;
        vertices[bufferIndex++] =(*iter)(1)+radius;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =1.f;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =0.f;
        
        vertices[bufferIndex++] =(*iter)(0)+radius;
        vertices[bufferIndex++] =(*iter)(1)-radius;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =1.f;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =0.f;
        
        vertices[bufferIndex++] =(*iter)(0)+radius;
        vertices[bufferIndex++] =(*iter)(1)+radius;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =1.f;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =0.f;
        
        vertices[bufferIndex++] =(*iter)(0)-radius;
        vertices[bufferIndex++] =(*iter)(1)+radius;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =1.f;
        vertices[bufferIndex++] =0.f;
        vertices[bufferIndex++] =0.f;
    }
    return vertices;
}
void Fluid::RenderFrame(){
    update();
    renderer->AddVertex(GenVertexArray(),listParticles.size()*6);
    renderer->RenderFrame();
}
void Fluid::updateDeltaTime()
{
	deltaTime = KCFL * CELL_WIDTH / maxVelocity;
	if (deltaTime > FRAME_TIME)
	{
		deltaTime = FRAME_TIME;
	}
}


void Fluid::updateCells(){
	unsigned int layer_size = sizeof(layer);
	memset(layer, 0, layer_size);
	for (list<Vector2f>::iterator iter = listParticles.begin(); iter != listParticles.end();)
	{
		int i = static_cast<int>(floor((*iter)(0) / CELL_WIDTH));
		int j = static_cast<int>(floor((*iter)(1) / CELL_WIDTH));
		if (status[i][j] == SOLID_WALL){
			
			iter = listParticles.erase(iter);
			continue;
		}
		else{
			status[i][j]=FLUID;
			layer[i][j]=1;
		}
		iter++;
	}
	for(int i=0;i<SIZE;i++)
		for(int j=0;j<SIZE;j++){
			if(status[i][j]!=SOLID_WALL&&!layer[i][j]){//·ÇÇ½·ÇÒº£¬¿ÕÆø
					status[i][j]=AIR;
			}
		}
}

Vector2f Fluid::getVelocity(double x, double y)
{
	Vector2f w;
	w(0) = getInterpolatedValue(x/CELL_WIDTH, y/CELL_WIDTH-0.5 , DIRECTION_X);
	w(1) = getInterpolatedValue(x/CELL_WIDTH-0.5, y/CELL_WIDTH , DIRECTION_Y);
	return w;
	
}

double Fluid::getInterpolatedValue(double x, double y, int direction)
{
    int i = static_cast<int>(floor(x));
	int j = static_cast<int>(floor(y));
	
	double weight = 0.0;
	double sum = 0.0;
	switch(direction)
	{
	case DIRECTION_X:
		if (status[i][j]!= SOLID_WALL)
		{
			sum += (i + 1 - x) * (j + 1 - y) * u[i][j];
			weight += (i + 1 - x) * (j + 1 - y);
		}
		if (status[i + 1][j] != SOLID_WALL)
		{
			sum += (x - i) * (j + 1 - y)  * u[i + 1][j];
			weight += (x - i) * (j + 1 - y);
		}
		if (status[i][j + 1] != SOLID_WALL)
		{
			sum += (i + 1 - x) * (y - j) * u[i][j + 1];
			weight += (i + 1 - x) * (y - j);
		}
		if (status[i + 1][j + 1]!= SOLID_WALL)
		{
			sum += (x - i) * (y - j) * u[i + 1][j + 1];
			weight += (x - i) * (y - j) ;
		}
        if (weight == 0.)
            return 0.;
        else
		    return sum / weight;
		break;
	case DIRECTION_Y:
		if (status[i][j]!= SOLID_WALL)
		{
			sum += (i + 1 - x) * (j + 1 - y) * v[i][j];
			weight += (i + 1 - x) * (j + 1 - y);
		}
		if (status[i + 1][j] != SOLID_WALL)
		{
			sum += (x - i) * (j + 1 - y) * v[i + 1][j];
			weight += (x - i) * (j + 1 - y);
		}
		if (status[i][j + 1] != SOLID_WALL)
		{
			sum += (i + 1 - x) * (y - j) * v[i][j + 1];
			weight += (i + 1 - x) * (y - j);
		}
		if (status[i + 1][j + 1] != SOLID_WALL)
		{
			sum += (x - i) * (y - j)  * v[i + 1][j + 1];
			weight += (x - i) * (y - j);
		}
        if(weight == 0.)
            return 0.;
        else
		    return sum / weight;
		break;
	default:
		return 0;
	}
}


Vector2f Fluid::traceParticle(double x, double y, double t)
{
	Vector2f w = getVelocity(x, y);
    w = getVelocity(x - 0.5 * t * w(0), y - 0.5 * t * w(1));
	return Vector2f(x,y) - w * t;
}

void Fluid::advection(){
	unsigned int u_size = sizeof(u),v_size = sizeof(v);
	memset(newu, 0, u_size);
	memset(newv, 0, v_size);//Initialization is important to avoid unexpected data input
	for (int i=0;i<SIZE;i++)
		for (int j=0;j<SIZE;j++){
			
			if(status[i][j]==FLUID){
				Vector2f w;
				w = this->traceParticle(static_cast<double>(i) * CELL_WIDTH, static_cast<double>(j + 0.5) * CELL_WIDTH, this->deltaTime);
				newu[i][j] = this->getVelocity(w(0),w(1))(0);
				w = this->traceParticle(static_cast<double>(i+ 0.5) * CELL_WIDTH, static_cast<double>(j) * CELL_WIDTH, this->deltaTime);
				newv[i][j] = this->getVelocity(w(0),w(1))(1);
			}
			else {
				if (i>0&&status[i-1][j] == FLUID){
				Vector2f w;
			    w = this->traceParticle((i+0.) * CELL_WIDTH, (j + 0.5) * CELL_WIDTH, deltaTime);
                    
				newu[i][j] = this->getVelocity(w(0),w(1))(0);
				
				}
				if (j>0&&status[i][j-1] == FLUID)
				{
				
				Vector2f w;
				w = this->traceParticle(static_cast<double>(i + 0.5) * CELL_WIDTH, static_cast<double>(j) * CELL_WIDTH, deltaTime);
				newv[i][j] = this->getVelocity(w(0),w(1))(1);
				
				}
			}
	}
    	for (int i=0;i<SIZE;i++)
		for (int j=0;j<SIZE;j++){
			if(status[i][j]!= SOLID_WALL){
				u[i][j]=newu[i][j];
				v[i][j]=newv[i][j];
			}
	}
}

void Fluid::addGravity(){

	for (int j=0;j<SIZE;j++)
		for (int i=0;i<SIZE;i++){
			if(status[i][j]==FLUID)
				v[i][j] -= deltaTime * GRAVITY;
			else if (j>0&&status[i][j-1] == FLUID)
				v[i][j] -= deltaTime * GRAVITY;
		}
}

void Fluid::getPressure(){
	vector<int> index_x;
	vector<int> index_y;
	int posX,posY;
	int fluid_count = 0;
	
  for (int i=0;i<SIZE;i++)
		for (int j=0;j<SIZE;j++){
			if (status[i][j] == FLUID)
			{
				index_x.push_back(i);
				index_y.push_back(j);
				layer[i][j]=fluid_count;
				fluid_count++;
			}
	}

  SpMat P(fluid_count, fluid_count);
  P.reserve(Eigen::VectorXi::Constant(fluid_count,5));
	int row_index = 0;
  for (int i=0;i<fluid_count;i++){
		int neighbor = 0;

		posX = index_x[i];
		posY = index_y[i];
		
    if (status[posX - 1][posY] == FLUID)
		{
			neighbor++;
      P.insert(row_index,layer[posX-1][posY]) = -1.0;
		}
		else if (status[posX - 1][posY] == AIR)
		{
			neighbor++;
		}

		if (status[posX + 1][posY] == FLUID)
		{
			neighbor++;
      P.insert(row_index,layer[posX+1][posY]) = -1.0;
		}
		else if (status[posX + 1][posY] == AIR)
		{
			neighbor++;
		}

		if (status[posX][posY - 1] == FLUID)
		{
			neighbor++;
      P.insert(row_index,layer[posX][posY-1]) = -1.0;
		}
		else if (status[posX][posY - 1] == AIR)
		{
			neighbor++;
		}

		if (status[posX][posY + 1] == FLUID)
		{
			neighbor++;
      P.insert(row_index,layer[posX][posY+1]) = -1.0;
		}
		else if (status[posX][posY + 1] == AIR)
		{
			neighbor++;
		}
	
    P.insert(row_index,row_index) = (double)neighbor;
      
		row_index++;
	}
    Eigen::VectorXd x(fluid_count), rhs(fluid_count);
	for (int i = 0; i < fluid_count; i++){
		  posX=index_x[i];
		  posY=index_y[i];
			rhs(i) = -(u[posX+1][posY]-u[posX][posY]+v[posX][posY+1]-v[posX][posY]);
			if ( status[posX-1][posY] == SOLID_WALL)
				rhs(i) -=u[posX][posY];
			if (status[posX+1][posY] == SOLID_WALL)
				rhs(i) +=u[posX+1][posY];

			if (status[posX][posY-1] == SOLID_WALL)
				rhs(i) -=v[posX][posY];
			if (status[posX][posY+1] == SOLID_WALL)
				rhs(i) +=v[posX][posY+1];
	}
  Eigen::ConjugateGradient<SpMat>	cg;
  cg.compute(P);
  x = cg.solve(rhs);
	for (int i = 0; i != fluid_count; i++)
	{
		posX=index_x[i];
		posY=index_y[i];
		p[posX][posY] = x(i);
	}
	
	maxVelocity = 0.0;
	for (int i=0;i<SIZE;i++)
		for (int j=0;j<SIZE;j++){
			if (status[i][j] == FLUID){
				u[i][j] -= p[i][j];
				u[i+1][j] +=p[i][j];
				v[i][j] -=p[i][j];
				v[i][j+1] +=p[i][j];
				double l = u[i][j]*u[i][j]+v[i][j]*v[i][j];
				if (maxVelocity*maxVelocity < l){
					maxVelocity = sqrt(l);
				}
			}
		}
    for (int i=0;i<SIZE;i++)
		for (int j=0;j<SIZE;j++){
			if (status[i][j] == SOLID_WALL){
				u[i][j]=0;
				u[i+1][j]=0;
				v[i][j]=0;
				v[i][j+1]=0;
			}
		    
	}
}


void Fluid::updateStep(){
	
	updateCells();
	
    advection();		

	addGravity();	

	getPressure();		
		
	moveParticles(deltaTime);
	
}
void Fluid::update(){
	remainderTime=FRAME_TIME;
	while (remainderTime>1e-8){
		    updateDeltaTime();		//1
			remainderTime -= deltaTime;
			if(remainderTime<-1e-8){
				deltaTime = deltaTime+remainderTime;
			    updateStep();
				break;
			}
			else
				updateStep();
	}	
}

void Fluid::moveParticles(float time)
{
	for (list<Vector2f>::iterator iter = listParticles.begin(); iter != listParticles.end();iter++)
	{
		Vector2f before = *iter;
		Vector2f start_velocity = getVelocity(before(0),before(1));
		Vector2f midpoint = before + start_velocity*0.5f*time;
		Vector2f mid_velocity = getVelocity(midpoint(0),midpoint(1));
		*iter += mid_velocity * time;
	}

}

