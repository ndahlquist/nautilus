#include <OpenMesh/Core/IO/Options.hh>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <Eigen/Core>
#include <iostream>
#include <fstream>
#include <sstream>
#include <GLUT/glut.h>
#include <time.h>
#include <stdlib.h>

//wall type codes
static const int NOTWALL = 0;
static const int XWALL = 1;
static const int YWALL = 2; 
static const int ZWALL = 3;
static const int FC = 4;
static const int CORE = 5;
static const int BORDER = 6;

using namespace Eigen;
using namespace std;
using namespace OpenMesh;

class Chamber;
class Chamber{
	private:
		int wallType;
		bool wall;
	public:
		Chamber* parent;
		int rank;
		Chamber* getRoot();
		Chamber();
		Chamber(int type);
		void setType(int t);
		int type();
		bool isWall();
};

class Wall {
	private:
	public:
		int sx; int sy; int sz;
		int ax; int ay; int az;
		int bx; int by; int bz;
		int type;
		Wall(int x, int y, int z, int which);
};

Wall::Wall(int x, int y, int z, int which){
	switch (which){
		case XWALL:
			ax = x-1; ay = y; az = z;
			bx = x+1; by = y; bz = z;
			break;
		case YWALL:
			ax = x; ay = y-1; az = z;
			bx = x; by = y+1; bz = z;
			break;
		case ZWALL:
			ax = x; ay = y; az = z-1;
			bx = x; by = y; bz = z+1;
			break;
		default:
			break;
	}
	sx = x; sy = y; sz = z;
	type = which;
}

void Chamber::setType(int t){
	(t == NOTWALL) ? wall = false : wall = true;
	wallType = t;
}

int Chamber::type(){
	return wallType;
}

bool Chamber::isWall(){
	return wall;
}

Chamber::Chamber(){
	rank = 0;
	parent = this;
	wall = false;
	wallType = NOTWALL;
}

Chamber::Chamber(int type){
	rank = 0;
	parent = this;
	type == NOTWALL ? wall = false : wall = true;
	wallType = type;
}

Chamber* find(Chamber* c) {
	if (c != c->parent){
		c->parent = find(c->parent);
	} 
	return c->parent;
}

Chamber* Chamber::getRoot() {
	return find(this);
}

void cUnion(Chamber* a, Chamber* b) {
	Chamber* rootA = find(a); Chamber* rootB = find(b);
	if (rootA != rootB){
		if (rootA->rank > rootB->rank){
			rootB->parent = rootA;
		} else {
			rootA->parent = rootB;
			if (rootA->rank == rootB->rank){
				++(rootB->rank);
			}
		}
	}
}
