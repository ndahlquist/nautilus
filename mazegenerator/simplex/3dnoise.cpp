//3d Simplex Noise
#include <math.h>
#include <stdlib.h>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include "3dnoise.h"

using namespace std;
using namespace OpenMesh;

//Following implementation as laid out by Stefan Gustavson
//in Simplex Noise Demystified

static const Vec3f grad[12] = {
	Vec3f(1,1,0), Vec3f(-1,1,0), Vec3f(1,-1,0), Vec3f(-1,-1,0),
	Vec3f(1,0,1), Vec3f(-1,0,1), Vec3f(1,0,-1), Vec3f(-1,0,-1),
	Vec3f(0,1,1), Vec3f(0,-1,1), Vec3f(0,1,-1), Vec3f(0,-1,-1),
};


int permutationTable[512]; //doubled to avoid wrapping
int repeatThresh = 256;

void initNoise(float seed) {
	srand(seed);
	for (int i = 0; i < repeatThresh; i++){
		permutationTable[i] = i; 
	}
	for (int i = 0; i < repeatThresh; i++){ //randomize order but maintain only 0-255 values
		int temp = permutationTable[i];
		int swapWith = rand()%repeatThresh;
		permutationTable[i] = permutationTable[swapWith];
		permutationTable[swapWith] = temp;
	}
	for (int i = repeatThresh; i < repeatThresh*2; i++){
		permutationTable[i] = permutationTable[i%repeatThresh];
	}
}

int pInt(int x, int y, int z) { //return psuedorandom int thru hashing
	return permutationTable[(permutationTable[(permutationTable[z] + y)] + x)];
}

static int fastfloor(double x){
	if (x > 0){
		return (int)x;
	} else {
		return (int)(x - 1);
	}
}


double noise(double x, double y, double z, int k) { //k is the octave of the desired noise
	int wavelen = 2 << k;
	x *= wavelen; y *= wavelen; z *= wavelen;
	double skew = (x+y+z) * 1/3;
	
	int i0, j0, k0, i1, j1, k1, i2, j2, k2, i3, j3, k3; //corners of surrounding simplex
	i3 = 1; j3 = 1; k3 = 1;
	i0 = fastfloor(x + skew); j0 = fastfloor(y + skew); k0 = fastfloor(z + skew);
	double unskew = (i0+j0+k0) * 1/6; //unskew factor
	double x0 = x - (i0 - unskew); double y0 = y - (j0 - unskew); double z0 = z - (k0 - unskew);

	if(x0 > y0){
		if(y0 >= z0){
			i1 = 1; j1 = 0; k1 = 0;
			i2 = 1; j2 = 1; k2 = 0;
		} else if(x0 >= z0){
			i1 = 1; j1 = 0; k1 = 0;
			i2 = 1; j2 = 0; k2 = 1; 
		} else {
			i1 = 0; j1 = 0; k1 = 1;
			i2 = 1; j2 = 0; k2 = 1;
		}
	} else {
		if(y0 < z0){
			i1 = 0; j1 = 0; k1 = 1;
			i2 = 0; j2 = 1; k2 = 1;
		} else if(x0 < z0){
			i1 = 0; j1 = 1; k1 = 0;
			i2 = 0; j2 = 1; k2 = 1; 
		} else {
			i1 = 0; j1 = 1; k1 = 0;
			i2 = 1; j2 = 1; k2 = 0;
		}
	}
	//gradient indices  
	// From the paper: A step of (1,0,0) in (i,j,k) means a step of (1-c,-c,-c) in (x,y,z),
	// a step of (0,1,0) in (i,j,k) means a step of (-c,1-c,-c) in (x,y,z), and
	// a step of (0,0,1) in (i,j,k) means a step of (-c,-c,1-c) in (x,y,z), where
	// c = 1/6.
	double x1, x2, x3, y1, y2, y3, z1, z2, z3;
	x1 = x0 - i1 + 1/6; x2 = x0 - i2 + 2/6; x3 = x0 - i3 + 1/2;
	y1 = y0 - j1 + 1/6; y2 = y0 - j2 + 2/6; y3 = y0 - j3 + 1/2;
	z1 = z0 - k1 + 1/6; z2 = z0 - k2 + 2/6; z3 = z0 - k3 + 1/2;

	int gi0 = pInt(i0&255, j0&255, k0&255) % 12;
 	int gi1 = pInt(i0&255 + i1, j0&255 + j1, k0&255 + k1) % 12;
 	int gi2 = pInt(i0&255 + i2, j0&255 + j2, k0&255 + k2) % 12;
 	int gi3 = pInt(i0&255 + i3, j0&255 + j3, k0&255 + k3) % 12;

 	//contribution from four corners
 	double n0, n1, n2, n3; 
 	double t0 = 0.5 - x0*x0 - y0*y0 - z0*z0;
	if (t0 < 0){
		n0 = 0;
	} else {
		t0 *= t0;
		n0 = t0 * t0 * dot(grad[gi0], Vec3f(x0,y0,z0));
	}
	double t1 = 0.5 - x1*x1 - y1*y1 - z1*z1;
	if(t1 < 0) {
		n1 = 0;
	} else {
		t1 *= t1;
		n1 = t1 * t1 * dot(grad[gi1], Vec3f(x1,y1,z1));
	}
	double t2 = 0.5 - x2*x2 - y2*y2 - z2*z2;
	if(t2 < 0) {
		n2 = 0.0;
	} else {
		t2 *= t2;
		n2 = t2 * t2 * dot(grad[gi2], Vec3f(x2,y2,z2));
	}
	double t3 = 0.5 - x3*x3 - y3*y3 - z3*z3;
	if (t3 < 0) {
		n3 = 0.0;
	} else {
		t3 *= t3;
		n3 = t3 * t3 * dot(grad[gi3], Vec3f(x3,y3,z3));
	}
	return 32.0*(n0 + n1 + n2 + n3); //add contributions and scale
}

double composedNoise(double x, double y, double z, int octaves, int p, int baseFreq) {
	double value = 0;
	double amp = 1;
	double scale = 0;
	for (int i = 0; i < octaves; i++){
		value += noise(x*baseFreq, y*baseFreq, z*baseFreq, i) * amp;
		scale += amp;
		amp *= p;
	}
	return value/scale;
}