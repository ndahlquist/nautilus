// obj_parser.h
// nativeGraphics

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <vector>
#include <cstring>

#include "Vector3.h"
#include "Point3.h"

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

static void parseObjString(char * objString, std::vector<struct Vertex> & vertices, std::vector<struct face> & faces);
static void computeAdjacencyLists(std::vector<struct Vertex> & vertices, std::vector<struct face> & faces);
static void subdivideMesh(std::vector<struct Vertex> & vertices, std::vector<struct face> & faces);
static void smoothMesh(std::vector<struct Vertex> & vertices, std::vector<struct face> & faces);
static void computeNormals(std::vector<struct Vertex> & vertices, std::vector<struct face> & faces);

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

struct subdividedFace {
	subdividedFace() {
		vertex[0] = -1;
		vertex[1] = -1;
		vertex[2] = -1;
	}
	int vertex[3];
};

static void subdivideMesh(std::vector<struct Vertex> & vertices, std::vector<struct face> & faces) {

	LOGI("Beginning subdivision with %d faces and %d vertices", faces.size(), vertices.size());

    // Create a second face array
	std::vector<subdividedFace> subdividedfaces;
	for(int i = 0; i < faces.size(); i++) {
		struct subdividedFace newFace;
		newFace.vertex[0] = -1;
		newFace.vertex[1] = -1;
		newFace.vertex[2] = -1;
		subdividedfaces.push_back(newFace);
	}
	
	// Reset all vertices to even
	for(int i = 0; i < vertices.size(); i++)
	    vertices[i].odd = false;

	for(int i = 0; i < faces.size(); i++) {
		for(int j = 0; j < 3; j++) {
		
			struct Vertex * oldVertex1 = &vertices[faces[i].vertex[(j+1)%3]];
			struct Vertex * oldVertex2 = &vertices[faces[i].vertex[(j+2)%3]];
			Point3 newCoord = Point3::Lerp(oldVertex1->coord, oldVertex2->coord, .5f);

			// See if we can reuse an existing midpoint
			bool matchFound = false;
			for(int k = 0; k < vertices.size(); k++) {
			    if(vertices[k].coord == newCoord) {
					matchFound = true;
					subdividedfaces[i].vertex[j] = k;
					break; // TODO
				}
			}
			/*for(int k = 0; k < 3; k++) {
			    for(int l = 0; l < 3; l++) { // TODO
			    int adjacentFaceIndex = faces[i].adjacentFace[l];
				int potentialIndex = subdividedfaces[adjacentFaceIndex].vertex[k];
				if(potentialIndex != -1 && coordsMatch(vertices[potentialIndex].coord, newVertex)) {
					LOGI("Match found");
					matchFound = true;
					subdividedfaces[i].vertex[j] = potentialIndex;
				}
				}
			}*/

			if(!matchFound) {
			    Vertex newVertex = Vertex(newCoord.x, newCoord.y, newCoord.z);
			    newVertex.odd = true;
				vertices.push_back(newVertex);
				subdividedfaces[i].vertex[j] = vertices.size() - 1;
				LOGI("Created new vertice %d", vertices.size() - 1);
			}
		}
	}

	LOGI("Subdivision sub-tesselation complete");

	std::vector<struct face> newFaces;
	for(int i = 0; i < faces.size(); i++) {

		// Add the three new outer faces
		for(int j = 0; j<3; j++) {
			int vertexJ = faces[i].vertex[j];
			int vertexJ1 = subdividedfaces[i].vertex[(j+1)%3];
			int vertexJ2 = subdividedfaces[i].vertex[(j+2)%3];
			if(vertexJ < 0 || vertexJ1 < 0 || vertexJ2 < 0)
				LOGE("Error: subdivideMesh caused an unitialized vertex.");
			newFaces.push_back(face(vertexJ, vertexJ1, vertexJ2));
		}

		// Add the new inner face
		int vertexJ1 = subdividedfaces[i].vertex[2];
		int vertexJ2 = subdividedfaces[i].vertex[1];
		int vertexJ3 = subdividedfaces[i].vertex[0];
		if(vertexJ1 < 0 || vertexJ2 < 0 || vertexJ3 < 0)
			LOGE("Error: subdivideMesh caused an unitialized vertex.");
		newFaces.push_back(face(vertexJ1, vertexJ2, vertexJ3));
	}

	LOGI("Subdivision complete, a total of %d faces and %d vertices", newFaces.size(), vertices.size());

	faces = newFaces;
}

	static std::vector<int> getAdjacentVertices(int index, std::vector<struct Vertex> & vertices, std::vector<struct face> & faces) {
	    std::vector<int> adjacentVertices;
		for(int i = 0; i < vertices[index].adjacentFaces.size(); i++) {
		    for(int j = 0; j < 3; j++) {
		        int candidate = faces[vertices[index].adjacentFaces[i]].vertex[j];
		        if(candidate == index)
		            continue;
		        adjacentVertices.push_back(candidate);
		    }
		}
	    return adjacentVertices;
	}

static void smoothMesh(std::vector<struct Vertex> & vertices, std::vector<struct face> & faces) {
	std::vector<Vertex> originalVertices = std::vector<Vertex>(vertices);
	for(int i = 0; i < vertices.size(); i++) {
		float total[3] = {0,0,0};
		struct Vertex * thisVertex = &originalVertices[i];
		if(thisVertex->odd) {
		    /*for(int j = 0; j < thisVertex->adjacentFaces.size(); j++) {
			    for(int k = 0; k < 3; k++) {
				    struct Vertex * neighborVertex = &originalVertices[faces[thisVertex->adjacentFaces[j]].vertex[k]];
				    total[0] += neighborVertex->coord.x;
				    total[1] += neighborVertex->coord.y;
				    total[2] += neighborVertex->coord.z;
				    denominator++;
			    }
		    }
		
		    if(denominator == 0) {
			    LOGE("Error: No neighbors detected for a vertex");
			    return;
		    }
		    total[0] /= denominator;
		    total[1] /= denominator;
		    total[2] /= denominator;
		    vertices[i].coord = Point3(total[0], total[1], total[2]);*/
		} else { // even
		    float beta = 3.0f/8.0f;
		    std::vector<int> adjacentVertices = getAdjacentVertices(i, vertices, faces);
		    if(adjacentVertices.size() > 3)
		        beta /= (float) adjacentVertices.size();
		    for(int j = 0; j < adjacentVertices.size(); j++) {
		        total[0] += beta * vertices[adjacentVertices[j]].coord.x;
		        total[1] += beta * vertices[adjacentVertices[j]].coord.y;
		        total[2] += beta * vertices[adjacentVertices[j]].coord.z;
		    }
		    
		    total[0] += (1.0f - beta * adjacentVertices.size()) * thisVertex->coord.x;
		    total[1] += (1.0f - beta * adjacentVertices.size()) * thisVertex->coord.y;
		    total[2] += (1.0f - beta * adjacentVertices.size()) * thisVertex->coord.z;
		    
		    thisVertex->coord = Point3(total[1], total[2], total[3]);
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
				if(texIndex >= textures.size())
					LOGE("texIndex %d out of bounds [0, %d]", texIndex, textures.size());
				vertices[faces[i].vertex[v]].texture[0] = textures[texIndex].coord[0];
				vertices[faces[i].vertex[v]].texture[1] = textures[texIndex].coord[1];
			}
		}
	}
}

#undef LOG_TAG
