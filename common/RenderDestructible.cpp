//
//  RenderTest.cpp
//  nativeGraphics
//
//  Created by Ling-Ling Zhang on 6/2/13.
//  Copyright (c) 2013 Ling-Ling Zhang. All rights reserved.
//

#include "RenderDestructible.h"
#include "glsl_helper.h"
#include "transform.h"
#include "common.h"
#include "log.h"

#include "Vector3.h"
#include "Point3.h"
#include "obj_parser.h"

#define voxelSize 20.0

struct DestructibleBond;
struct DestructibleNode;
struct DestructibleCell;
struct DestructibleFace;

struct DestructibleNode
{
    DestructibleNode()
    {
        position.x = 0;
        position.y = 0;
        position.z = 0;
    }
    DestructibleNode(float x, float y, float z)
    {
        position.x = x;
        position.y = y;
        position.z = z;
    }
    Point3 position;
    Vector3 velocity;
    vector<struct DestructibleBond *> bonds;
    int node_num;
};

struct DestructibleFace
{
    DestructibleFace(DestructibleNode *node1, DestructibleNode *node2, DestructibleNode *node3, DestructibleCell *c)
    {
        nodes.push_back(node1);
        nodes.push_back(node2);
        nodes.push_back(node3);
        cell = c;
    }
    DestructibleFace(DestructibleNode *node1, DestructibleNode *node2, DestructibleNode *node3)
    {
        nodes.push_back(node1);
        nodes.push_back(node2);
        nodes.push_back(node3);
    }
    vector<DestructibleNode *> nodes;
    DestructibleCell *cell;
};

struct DestructibleBond
{
    vector<DestructibleNode *> nodes;
    vector<DestructibleCell *> cells;
    GLfloat origLength;
    GLfloat springConst;
    GLfloat dampConst;
    GLfloat breakThresh;
    bool broken;
};

struct DestructibleCell
{
    vector<struct DestructibleBond *> bonds;
    vector<struct DestructibleNode *> nodes;
    bool broken;
    int idx;
};

static struct DestructibleCell* createCell(float xmin, float ymin, float zmin);
static struct DestructibleNode *createNode(float x, float y, float z);
static struct DestructibleBond *createBond(DestructibleNode *node1, DestructibleNode *node2);
static struct DestructibleFace *createFace(DestructibleNode *node1, DestructibleNode *node2, DestructibleNode *node3, DestructibleCell *cell);
static void createFragment(DestructibleNode *node);

std::vector<struct DestructibleCell *> cells;
std::vector<struct DestructibleNode *> nodes;
std::vector<struct DestructibleBond *> bonds;
std::vector<struct DestructibleFace *> surfaces;
std::vector<struct DestructibleFace *> fragments;
int node_counter = 0;
int cell_counter = 0;
static void parseObjString(char * line);

RenderDestructible::RenderDestructible(const char *objFilename, const char *vertexShaderFilename, const char *fragmentShaderFilename) : RenderObject(objFilename, vertexShaderFilename, fragmentShaderFilename) {
    
    nodes.erase(nodes.begin(), nodes.end());
    bonds.erase(bonds.begin(), bonds.end());
    surfaces.erase(surfaces.begin(), surfaces.end());
    fragments.erase(fragments.begin(), fragments.end());
    cells.erase(cells.begin(), cells.end());
    explode = false;
    
    parseObjString((char *)loadResource("subvox.obj"));
}

static void parseObjLine(char * line) {
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
        
		nodes.push_back(new DestructibleNode(x, y, z));
	}
    
	// Parse a face
	if(strcmp(tok, "f") == 0) {
		tok = strtok_r(NULL, " ", &saveptr);
		if(tok == NULL || tok[0] == '#') {
			LOGE("Error in ParseObjLine");
			return;
		}
		int v1 = atoi(tok); // Zero index
        
		char*texsaveptr;
		int texIndex[3] = {-1,-1,-1};
		strtok_r(tok, "/", &texsaveptr);
		char * textok = strtok_r(NULL, "/", &texsaveptr);
		if(textok != NULL)
			texIndex[0] = atoi(textok);
        
		tok = strtok_r(NULL, " ", &saveptr);
		if(tok == NULL || tok[0] == '#') {
			LOGE("Error in ParseObjLine");
			return;
		}
		int v2 = atoi(tok);
        
		strtok_r(tok, "/", &texsaveptr);
		textok = strtok_r(NULL, "/", &texsaveptr);
		if(textok != NULL)
			texIndex[1] = atoi(textok);
        
		tok = strtok_r(NULL, " ", &saveptr);
		if(tok == NULL || tok[0] == '#') {
			LOGE("Error in ParseObjLine");
			return;
		}
		int v3 = atoi(tok);
        
        tok = strtok_r(NULL, " ", &saveptr);
        if(tok == NULL || tok[0] == '#') {
            LOGE("Error in ParseObjLine");
            return;
        }
        int c = atoi(tok);
        
		strtok_r(tok, "/", &texsaveptr);
		textok = strtok_r(NULL, "/", &texsaveptr);
		if(textok != NULL)
			texIndex[2] = atoi(textok);
        
		/*struct face thisFace = face(v1, v2, v3);
         thisFace.texture[0] = texIndex[0];
         thisFace.texture[1] = texIndex[1];
         thisFace.texture[2] = texIndex[2];*/
		surfaces.push_back(new DestructibleFace(nodes[v1], nodes[v2], nodes[v3], cells[c]));
    }
    
    //Parse a bond
    if(strcmp(tok, "b") == 0) {
        tok = strtok_r(NULL, " ", &saveptr);
        if(tok == NULL || tok[0] == '#') {
            LOGE("Error in ParseObjLine");
            return;
        }
        float n1 = atof(tok);
        
        tok = strtok_r(NULL, " ", &saveptr);
        if(tok == NULL || tok[0] == '#') {
            LOGE("Error in ParseObjLine");
            return;
        }
        float n2 = atof(tok);
        DestructibleNode *node1 = nodes[n1];
        DestructibleNode *node2 = nodes[n2];
        DestructibleBond *bond = createBond(node1, node2);
        bonds.push_back(bond);
    }
    
    // Parse a cell
    if(strcmp(tok, "cn") == 0) {
        vector<int> node_idxs;
        tok = strtok_r(NULL, " ", &saveptr);
        if(tok == NULL || tok[0] == '#') {
            LOGE("Error in ParseObjLine");
            return;
        }
        node_idxs.push_back(atoi(tok));
        
        tok = strtok_r(NULL, " ", &saveptr);
        if(tok == NULL || tok[0] == '#') {
            LOGE("Error in ParseObjLine");
            return;
        }
        node_idxs.push_back(atoi(tok));
        tok = strtok_r(NULL, " ", &saveptr);
        if(tok == NULL || tok[0] == '#') {
            LOGE("Error in ParseObjLine");
            return;
        }
        node_idxs.push_back(atoi(tok));
        tok = strtok_r(NULL, " ", &saveptr);
        if(tok == NULL || tok[0] == '#') {
            LOGE("Error in ParseObjLine");
            return;
        }
        node_idxs.push_back(atoi(tok));
        tok = strtok_r(NULL, " ", &saveptr);
        if(tok == NULL || tok[0] == '#') {
            LOGE("Error in ParseObjLine");
            return;
        }
        node_idxs.push_back(atoi(tok));
        tok = strtok_r(NULL, " ", &saveptr);
        if(tok == NULL || tok[0] == '#') {
            LOGE("Error in ParseObjLine");
            return;
        }
        node_idxs.push_back(atoi(tok));
        tok = strtok_r(NULL, " ", &saveptr);
        if(tok == NULL || tok[0] == '#') {
            LOGE("Error in ParseObjLine");
            return;
        }
        node_idxs.push_back(atoi(tok));
        tok = strtok_r(NULL, " ", &saveptr);
        if(tok == NULL || tok[0] == '#') {
            LOGE("Error in ParseObjLine");
            return;
        }
        node_idxs.push_back(atoi(tok));
        
        DestructibleCell *cell = new DestructibleCell;
        
        for (int i = 0; i < node_idxs.size(); i++) {
            DestructibleNode *node = nodes[node_idxs[i]];
            cell->nodes.push_back(node);
            
            for (int j = 0; j < node->bonds.size(); j++) {
                DestructibleBond *bond = node->bonds[j];
                DestructibleNode *node2 = bond->nodes[0];
                if (node == node2)
                    node2 = bond->nodes[1];
                
                for (int k = i; k < node_idxs.size(); k++) {
                    if (node2 == nodes[node_idxs[k]]) {
                        cell->bonds.push_back(bond);
                        bond->cells.push_back(cell);
                    }
                }
            }
        }
        cells.push_back(cell);
    }
    
}
static void parseObjString(char * objString) {
    
	char * saveptr;
	char * line = strtok_r(objString, "\n", &saveptr);
	while(line != NULL) {
		parseObjLine(line);
		line = strtok_r(NULL, "\n", &saveptr);
	}
}

static struct DestructibleBond *createBond(DestructibleNode *node1, DestructibleNode *node2) {
    
    DestructibleBond *bond = new DestructibleBond;
    bond->nodes.push_back(node1);
    bond->nodes.push_back(node2);
    
    bond->origLength = sqrt(pow(node1->position.x-node2->position.x,2) + pow(node1->position.y-node2->position.y,2) + pow(node1->position.z-node2->position.z,2));
    bond->breakThresh = (GLfloat)(rand() % 100)/10;
    bond->springConst = .5;
    bond->broken = false;
    bond->dampConst = 2;
    
    node1->bonds.push_back(bond);
    node2->bonds.push_back(bond);
    
    return bond;
}

 static void createFragment(DestructibleNode *node) {
 float x = node->position.x;
 float y = node->position.y;
 float z = node->position.z;
 
 DestructibleNode *node1 = new DestructibleNode(x, y, z);
 DestructibleNode *node2 = new DestructibleNode(x + voxelSize/2, y, z);
 DestructibleNode *node3 = new DestructibleNode(x, y - voxelSize/2, z);
 DestructibleNode *node4 = new DestructibleNode(x, y, z + voxelSize/2);
 
 //TODO: add rotational velocity
 node1->velocity = node->velocity;
 node2->velocity = node->velocity;
 node3->velocity = node->velocity;
 node4->velocity = node->velocity;
 
 DestructibleFace *face1 = new DestructibleFace(node1, node2, node3);
 DestructibleFace *face2 = new DestructibleFace(node1, node3, node4);
 DestructibleFace *face3 = new DestructibleFace(node2, node3, node4);
 DestructibleFace *face4 = new DestructibleFace(node1, node4, node2);
 
 fragments.push_back(face1);
 fragments.push_back(face2);
 fragments.push_back(face3);
 fragments.push_back(face4);
 }

GLfloat * RenderDestructible::getGeometry(int & num_vertices) {
    if (!explode) {
        nodes[rand()%nodes.size()]->velocity = Vector3((rand()%100)/50 - 1.0, (rand()%100)/50 - 1.0, (rand()%100)/50 - 1.0);
        nodes[rand()%nodes.size()]->velocity = Vector3((rand()%100)/50 - 1.0, (rand()%100)/50 - 1.0, (rand()%100)/50 - 1.0);
        nodes[rand()%nodes.size()]->velocity = Vector3((rand()%100)/50 - 1.0, (rand()%100)/50 - 1.0, (rand()%100)/50 - 1.0);
        explode = true;
    }
    
    for (int node_idx = 0; node_idx < nodes.size(); node_idx++) {
        DestructibleNode *node = nodes[node_idx];
        Vector3 force = Vector3();
        
        //Internal forces
        for (int bond_idx = 0; bond_idx < node->bonds.size(); bond_idx++) {
            DestructibleBond *bond = node->bonds[bond_idx];
            
            if (bond->broken) {
                node->bonds.erase(node->bonds.begin() + bond_idx);
                bond_idx--;
                continue;
            }
            
            DestructibleNode *node1 = bond->nodes[0];
            DestructibleNode *node2 = bond->nodes[1];
            
            Vector3 vec = Vector3(node1->position.x - node2->position.x, node1->position.y - node2->position.y, node1->position.z - node2->position.z);
            float bondLength = sqrt(pow(node1->position.x-node2->position.x,2) + pow(node1->position.y-node2->position.y,2) + pow(node1->position.z-node2->position.z,2));
            vec.Normalize();
            vec = vec * (bondLength - bond->origLength) * bond->springConst;
            
            Vector3 vel = (node1->velocity - node2->velocity) * bond->dampConst;
            
            if (node->position.x == node2->position.x && node->position.y == node2->position.y && node->position.z == node2->position.z) {
                vec *= -1;
                vel *= -1;
            }
            
            force = force + vec + vel;
        }
        Vector3 accel = force/ 2.0;
        node->velocity = node->velocity + (accel * .1);
        node->position = node->position + node->velocity *.1;
        
        if (node->bonds.size() == 0) {
            createFragment(node);
            nodes.erase(nodes.begin() + node_idx);
            node_idx--;
        }
    }
    
    for (int face_idx = 0; face_idx < fragments.size(); face_idx++) {
        DestructibleFace *face = fragments[face_idx];
        for (int i = 0; i < face->nodes.size(); i++) {
            face->nodes[i]->position = face->nodes[i]->position + (face->nodes[i]->velocity * .1);
        }
    }
    
    //Check for broken cells
    for (int i = 0; i < cells.size(); i++)
    {
        DestructibleCell *cell = cells[i];
        if (cell->broken)
            continue;
        
        for (int j = 0; j < cell->bonds.size(); j++) {
            DestructibleBond *bond = cell->bonds[j];
            DestructibleNode *node1 = bond->nodes[0];
            DestructibleNode *node2 = bond->nodes[1];
            float bondLength = sqrt(pow(node1->position.x-node2->position.x,2) + pow(node1->position.y-node2->position.y,2) + pow(node1->position.z-node2->position.z,2));
            if (bondLength/bond->origLength > 1.0 + bond->breakThresh) {
                cell->broken = true;
            }
            for (int k = 0; k < bond->cells.size(); k++) {
                if (bond->cells[k]->broken) {
                    bond->cells.erase(bond->cells.begin() + k);
                    k--;
                }
            }
        }
    }
    
    //Check for broken bonds
    for (int i = 0; i < bonds.size(); i++) {
        DestructibleBond *bond = bonds[i];
        for (int j = 0; j < bond->cells.size(); j++) {
            if (bond->cells[j]->broken) {
                bond->cells.erase(bond->cells.begin() + j);
                j--;
            }
        }
        if (bonds[i]->cells.size() == 0)
            bonds[i]->broken = true;
    }
    
    //Find surface faces
    std::vector<struct DestructibleNode *> surfaceNodes;
    int numSurfaceVertices = 0;
    
    for (int face_idx = 0; face_idx < surfaces.size(); face_idx++) {
        struct DestructibleFace * face = surfaces[face_idx];
        if (face->cell->broken) {
            surfaces.erase(surfaces.begin() + face_idx);
            face_idx--;
            continue;
        }
        
        bool surface = true;
        for (int node_idx = 0; node_idx < face->nodes.size(); node_idx++) {
            if (face->nodes[node_idx]->bonds.size() > 13) {
                surface = false;
                break;
            }
        }
        if (surface == true) {
            numSurfaceVertices += 3;
            surfaceNodes.push_back(face->nodes[0]);
            surfaceNodes.push_back(face->nodes[1]);
            surfaceNodes.push_back(face->nodes[2]);
        }
    }
    
    GLfloat * vertexBuffer = (float *)malloc((numSurfaceVertices + fragments.size()*3) * (3) * sizeof(float));
    int bufferIndex = 0;
    for (int node_idx = 0; node_idx < surfaceNodes.size(); node_idx++) {
        vertexBuffer[bufferIndex++] = surfaceNodes[node_idx]->position.x;
        vertexBuffer[bufferIndex++] = surfaceNodes[node_idx]->position.y;
        vertexBuffer[bufferIndex++] = surfaceNodes[node_idx]->position.z;
    }
    
    for (int face_idx = 0; face_idx < fragments.size(); face_idx++) {
        DestructibleFace *face = fragments[face_idx];
        for (int i = 0; i < face->nodes.size(); i++) {
            vertexBuffer[bufferIndex++] = face->nodes[i]->position.x;
            vertexBuffer[bufferIndex++] = face->nodes[i]->position.y;
            vertexBuffer[bufferIndex++] = face->nodes[i]->position.z;
        }
    }
    
    num_vertices = numSurfaceVertices + fragments.size()*3;
    return vertexBuffer;
}

void RenderDestructible::RenderPass(int instance, GLfloat *buffer, int num) {
    
    // Create vbo // TODO: Shouldn't be vertex buffered.
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, num * (3) * sizeof(float), buffer, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    checkGlError("VertexBuffer Generation");
    
    // Pass matrices
    GLfloat* mv_Matrix = (GLfloat*)mvMatrix();
    GLfloat* mvp_Matrix = (GLfloat*)mvpMatrix();
    glUniformMatrix4fv(gmvMatrixHandle, 1, GL_FALSE, mv_Matrix);
    glUniformMatrix4fv(gmvpMatrixHandle, 1, GL_FALSE, mvp_Matrix);
    checkGlError("glUniformMatrix4fv");
    delete[] mv_Matrix;
    delete[] mvp_Matrix;
    
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBuffer);
    
    // Pass vertices
    glEnableVertexAttribArray(gvPositionHandle);
    glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) 0);
    checkGlError("gvPositionHandle");

    // Pass texture
    if(textureUniform != -1 && texture != -1) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(textureUniform, 0);
        checkGlError("texture");
    }
    
    // Pass normal map
    if(normalMapUniform != -1 && normalTexture != -1) {
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalTexture);
        glUniform1i(textureUniform, 1);
        checkGlError("normalTexture");
    }
    
    glDrawArrays(GL_TRIANGLES, 0, num);
    checkGlError("glDrawArrays");
    
}

void RenderDestructible::Render() {
    
    if(!pipeline) {
        LOGE("RenderPipeline inaccessible.");
        exit(0);
    }
    
    int num_vertices;
    GLfloat * geometry = getGeometry(num_vertices);
    
    //////////////////////////////////
    // Render to frame buffer
    
    // Render colors (R, G, B, UNUSED / SPECULAR)
    SetShader(colorShader);
    
    glBindFramebuffer(GL_FRAMEBUFFER, pipeline->frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pipeline->colorTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pipeline->depthBuffer);
    
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL); // TODO: Measure effect on performance vs clear buffer.
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);
    checkGlError("glClear");
    
    RenderPass(0, geometry, num_vertices);
    
    // Render geometry (NX_MV, NY_MV, NZ_MV, Depth_MVP)
    SetShader(pipeline->geometryShader);
    
    glBindFramebuffer(GL_FRAMEBUFFER, pipeline->frameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pipeline->geometryTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pipeline->depthBuffer);
    
    glDepthMask(GL_FALSE); // We share the same depth buffer here, so don't overwrite it.
    glDepthFunc(GL_EQUAL);
    glDisable(GL_DITHER);
    
    RenderPass(0, geometry, num_vertices);
    
    free(geometry);
    
    glDepthMask(GL_TRUE); // TODO
    
    glBindBuffer(GL_ARRAY_BUFFER, 0); // TODO: unbind other resources
}
