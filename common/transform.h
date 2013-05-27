#ifndef __nativeGraphics__transform__
#define __nativeGraphics__transform__

#include <iostream>
#include "Eigen/Core"
#include <stack>
using Eigen::Matrix4f;
using Eigen::Vector3f;

extern std::stack<Matrix4f> model_view;
extern std::stack<Matrix4f> projection;


//Model-view
//get the current matrix
float* mvMatrix();
//Push
void mvPushMatrix();
//Pop
void mvPopMatrix();
//Load Identity
void mvLoadIdentity();
//Scale
void scalef(float s);
void scalef(float sx, float sy, float sz);
//Translate
void translatef(float x, float y, float z);
//Rotate, angle in degrees
void rotatef(float angle, float x, float y, float z);
//rotate
void rotate(float rx, float ry, float rz);
//Scale

/* Following gluLookAt implementation is adapted from the
 * Mesa 3D Graphics library. http://www.mesa3d.org
 */
void lookAt(float eyex, float eyey, float eyez,
	              float centerx, float centery, float centerz,
            float upx, float upy, float upz);
//projection
float* pMatrix();

float* pInverseMatrix();

//Push
void pPushMatrix();
//Pop
void pPopMatrix();
//Load Identity
void pLoadIdentity();
//frustum
void frustum(double left, double right, double bottom, double top, 
             double nearVal, double farVal );
//Perspective
void perspective(float fovy, float aspect,
                 float zNear, float zFar);
void viewport(int x, int y, int width, int height);
//return combined mvp matrix
float* mvpMatrix();
float* mvInverseMatrix();

#endif /* defined(__nativeGraphics__transform__) */

