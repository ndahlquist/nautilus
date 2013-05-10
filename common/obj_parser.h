// obj_parser.h
// nativeGraphics

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <cstring>

#include "Vector3.h"
#include "Point3.h"

// Public Interface: -----------------------------------------------------------

float * getInterleavedBuffer(char * objString, int & numVertices, bool normalCoords = false, bool textureCoords = false);

static void parseObjString(char * objString, std::vector<struct Vertex> & vertices, std::vector<struct face> & faces);
static void computeAdjacencyLists(std::vector<struct Vertex> & vertices, std::vector<struct face> & faces);
static void subdivideMesh(std::vector<struct Vertex> & vertices, std::vector<struct face> & faces);
static void smoothMesh(std::vector<struct Vertex> & vertices, std::vector<struct face> & faces);
static void computeNormals(std::vector<struct Vertex> & vertices, std::vector<struct face> & faces);

// Implementation: -------------------------------------------------------------

#define  LOG_TAG    "obj_parser"
#include "log.h"

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

float * getInterleavedBuffer(char * objString, int & numVertices, bool normalCoords, bool textureCoords) {

    // Parse obj file into vertices and faces
    std::vector<struct Vertex> vertices;
	std::vector<struct face> faces;
	parseObjString(objString, vertices, faces);

    // Compute adjacent vertices and normals
	computeAdjacencyLists(vertices, faces);
	computeNormals(vertices, faces);

    const int floatsPerFace = 3 + (normalCoords ? 3 : 0) + (textureCoords ? 2 : 0);
    float * interleavedBuffer = (float *) malloc(faces.size() * 3 * floatsPerFace * sizeof(float));
    numVertices = faces.size()*3;
    
    int bufferIndex = 0;
    for(int i = 0; i < faces.size(); i++) {
		for(int v = 0; v < 3; v++) {
		
			int vertexIndex = faces[i].vertex[v];
			if(vertexIndex < 0 || vertexIndex >= vertices.size()) { // TODO
				LOGE("vertexIndex %d out of bounds (0, %d)", vertexIndex, (int) vertices.size());
		    }
			
			struct Vertex * thisVertex = &vertices[vertexIndex];

			interleavedBuffer[bufferIndex++] = thisVertex->coord.x;
			interleavedBuffer[bufferIndex++] = thisVertex->coord.y;
			interleavedBuffer[bufferIndex++] = thisVertex->coord.z;
            
            if(normalCoords) {
			    interleavedBuffer[bufferIndex++] = thisVertex->normal.x;
			    interleavedBuffer[bufferIndex++] = thisVertex->normal.y;
			    interleavedBuffer[bufferIndex++] = thisVertex->normal.z;
			}
            
            if(textureCoords) {
                interleavedBuffer[bufferIndex++] = thisVertex->texture[0];
			    interleavedBuffer[bufferIndex++] = thisVertex->texture[1];
			}
		}
	}
	
	return interleavedBuffer;

}

static bool checkAdjacency(struct face * a, struct face * b) {
	int sharedVertices = 0;
	for(int i=0; i<3; i++) {
		for(int j=0; j<3; j++) {
			if(a->vertex[i] == b->vertex[j])
				sharedVertices++;
		}
	}
	return sharedVertices == 2;
}

static void computeAdjacencyLists(std::vector<struct Vertex> & vertices, std::vector<struct face> & faces) {
	// Clear adjacency lists
	for(int i = 0; i < vertices.size(); i++)
		vertices[i].adjacentFaces.clear();
	for(int i = 0; i < faces.size(); i++)
		for(int j=0; j<3; j++)
			faces[i].adjacentFace[j] = -1;

	// Populate each vertex with its adjacent faces.
	for(int i = 0; i < faces.size(); i++) {
		struct face * thisFace = &faces[i];
		for(int j=0; j<3; j++) {
			int thisIndex = thisFace->vertex[j];
			struct Vertex * thisVertex = &vertices[thisIndex];
			if(thisVertex->isAdjacent(i))
				continue;
			thisVertex->adjacentFaces.push_back(i);
		}
	}
	
	// Populate each face with its adjacent faces.
	for(int i=0; i < faces.size(); i++) {
		struct face * thisFace = &faces[i];
		for(int j=0; j<3; j++) {
			int thisIndex = thisFace->vertex[j];
			struct Vertex * thisVertex = &vertices[thisIndex];
			for(int k=0; k<thisVertex->adjacentFaces.size(); k++) {
				struct face * possibleAdjacentFace = &faces[thisVertex->adjacentFaces[k]];
				if(checkAdjacency(thisFace, possibleAdjacentFace))
					thisFace->adjacentFace[j] = thisVertex->adjacentFaces[k];
			}
		}
	}
}

static void computeNormals(std::vector<struct Vertex> & vertices, std::vector<struct face> & faces) {
	// Calculate normals per-face
	for(int i = 0; i < faces.size(); i++) {
		struct face * thisFace = &faces[i];
		Point3 vertex0 = vertices[thisFace->vertex[0]].coord;
		Point3 vertex1 = vertices[thisFace->vertex[1]].coord;
		Point3 vertex2 = vertices[thisFace->vertex[2]].coord;
		Vector3 normal = Vector3::Cross(vertex1 - vertex0, vertex2 - vertex0);
		normal.Normalize();
		thisFace->normal = Point3(normal);
	}
	// Average faces for per-vertex normals
	for(int i = 0; i < vertices.size(); i++) {
		struct Vertex * thisVertex = &vertices[i];
		Vector3 totalNormal = Vector3();
		for(int j = 0; j < thisVertex->adjacentFaces.size(); j++) {
			struct face * thisFace = &faces[thisVertex->adjacentFaces[j]];
			totalNormal.x += thisFace->normal.x;
			totalNormal.y += thisFace->normal.y;
			totalNormal.z += thisFace->normal.z;
		}
		totalNormal.Normalize();
		thisVertex->normal = Point3(totalNormal);
	}
}

static void parseObjLine(char * line, std::vector<struct Vertex> & vertices, std::vector<struct Texture> & textures, std::vector<struct face> & faces) {
	char * saveptr;
	char * tok = strtok_r(line, " ", &saveptr);
	if(tok == NULL || tok[0] == '#')
		return;

	// Parse a vertex
	if(strcmp(tok, "v") == 0) {
		tok = strtok_r(NULL, " ", &saveptr);
		if(tok == NULL || tok[0] == '#') {
			LOGE("Error in ParseObjLine");
			return;
		}
		float x = atof(tok);

		tok = strtok_r(NULL, " ", &saveptr);
		if(tok == NULL || tok[0] == '#') {
			LOGE("Error in ParseObjLine");
			return;
		}
		float y = atof(tok);

		tok = strtok_r(NULL, " ", &saveptr);
		if(tok == NULL || tok[0] == '#') {
			LOGE("Error in ParseObjLine");
			return;
		}
		float z = atof(tok);

		vertices.push_back(Vertex(x, y, z));
	}

	if(strcmp(tok, "vt") == 0) {
		tok = strtok_r(NULL, " ", &saveptr);
		if(tok == NULL || tok[0] == '#') {
			LOGE("Error in ParseObjLine");
			return;
		}
		float u = atof(tok);

		tok = strtok_r(NULL, " ", &saveptr);
		if(tok == NULL || tok[0] == '#') {
			LOGE("Error in ParseObjLine");
			return;
		}
		float v = atof(tok);

		textures.push_back(Texture(u, v));
	}

	// Parse a face
	if(strcmp(tok, "f") == 0) {
		tok = strtok_r(NULL, " ", &saveptr);
		if(tok == NULL || tok[0] == '#') {
			LOGE("Error in ParseObjLine");
			return;
		}
		int v1 = atoi(tok) - 1; // Zero index

		char*texsaveptr;
		int texIndex[3] = {-1,-1,-1};
		strtok_r(tok, "/", &texsaveptr);
		char * textok = strtok_r(NULL, "/", &texsaveptr);
		if(textok != NULL)
			texIndex[0] = atoi(textok) - 1;

		tok = strtok_r(NULL, " ", &saveptr);
		if(tok == NULL || tok[0] == '#') {
			LOGE("Error in ParseObjLine");
			return;
		}
		int v2 = atoi(tok) - 1;

		strtok_r(tok, "/", &texsaveptr);
		textok = strtok_r(NULL, "/", &texsaveptr);
		if(textok != NULL)
			texIndex[1] = atoi(textok) - 1;

		tok = strtok_r(NULL, " ", &saveptr);
		if(tok == NULL || tok[0] == '#') {
			LOGE("Error in ParseObjLine");
			return;
		}
		int v3 = atoi(tok) - 1;

		strtok_r(tok, "/", &texsaveptr);
		textok = strtok_r(NULL, "/", &texsaveptr);
		if(textok != NULL)
			texIndex[2] = atoi(textok) - 1;

		struct face thisFace = face(v1, v2, v3);
		thisFace.texture[0] = texIndex[0];
		thisFace.texture[1] = texIndex[1];
		thisFace.texture[2] = texIndex[2];
		faces.push_back(thisFace);
	}
}

static void parseObjString(char * objString, std::vector<struct Vertex> & vertices, std::vector<struct face> & faces) {
	LOGI("parseObjString");

	std::vector<struct Texture> textures;

	char * saveptr;
	char * line = strtok_r(objString, "\n", &saveptr);
	while(line != NULL) {
		parseObjLine(line, vertices, textures, faces);
		line = strtok_r(NULL, "\n", &saveptr);
	}
	// Copy textures into vertex structs.
	for(int i = 0; i < faces.size(); i++) {
		for(int v = 0; v < 3; v++) {
			int texIndex = faces[i].texture[v];
			if(texIndex == -1) {
				vertices[faces[i].vertex[v]].texture[0] = 0;
				vertices[faces[i].vertex[v]].texture[1] = 0;
			} else {
				//if(texIndex < 0 || texIndex >= (int) textures.size())
				//	LOGE("texIndex %d out of bounds [0, %d]", texIndex, (int) textures.size()); // TODO: Why does this break on Linux?
				vertices[faces[i].vertex[v]].texture[0] = textures[texIndex].coord[0];
				vertices[faces[i].vertex[v]].texture[1] = textures[texIndex].coord[1];
			}
		}
	}
}

#undef LOG_TAG
