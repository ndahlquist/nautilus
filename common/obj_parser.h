// obj_parser.h
// nativeGraphics

#ifndef __nativeGraphics__obj_parser__
#define __nativeGraphics__obj_parser__

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <cstring>

#include "Vector3.h"
#include "Point3.h"
#include "log.h"

// Public Interface: -----------------------------------------------------------

float * getInterleavedBuffer(char * objString, int & numVertices, bool normalCoords = false, bool textureCoords = false);
void getObjectData(char * objString, int & numVertices, std::vector<struct Vertex> & vertices, std::vector<struct face> &faces, bool normalCoords = false, bool textureCoords = false);

static void parseObjString(char * objString, std::vector<struct Vertex> & vertices, std::vector<struct face> & faces);
static void computeAdjacencyLists(std::vector<struct Vertex> & vertices, std::vector<struct face> & faces);
static void subdivideMesh(std::vector<struct Vertex> & vertices, std::vector<struct face> & faces);
static void smoothMesh(std::vector<struct Vertex> & vertices, std::vector<struct face> & faces);
static void computeNormals(std::vector<struct Vertex> & vertices, std::vector<struct face> & faces);

// Implementation: -------------------------------------------------------------

struct Vertex {
	Vertex(float x, float y, float z) {
		coord = Point3(x, y, z);
		normal = Point3();
		odd = false;
	};
	Point3 coord;
	float texture[2];
	Point3 normal;
	std::vector<int> adjacentFaces;
	bool odd;

	bool isAdjacent(int face) {
		for(int i = 0; i<adjacentFaces.size(); i++)
			if(adjacentFaces[i] == face)
				return true;
		return false;
	}
};

struct Texture {
	Texture(float u, float v) {
		coord[0] = u;
		coord[1] = v;
	};
	float coord[2];
};

struct face {
	face(int v0, int v1, int v2) {
		vertex[0] = v0;
		vertex[1] = v1;
		vertex[2] = v2;
		adjacentFace[0] = -1;
		adjacentFace[1] = -1;
		adjacentFace[2] = -1;
	}
	int vertex[3];
	int texture[3];
	int adjacentFace[3];
	Point3 normal;
};


#endif // __nativeGraphics__obj_parser__
