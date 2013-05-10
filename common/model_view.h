#ifndef __nativeGraphics__model_view__
#define __nativeGraphics__model_view__

#include <iostream>
#include "Eigen/Core"
#include <stack>
using Eigen::Matrix4f;
using Eigen::Vector4f;

std::stack<Matrix4f> model_view;

//Push
void pushMatrix(){
  model_view.push(model_view.top());
}
//Pop
void popMatrix(){
  model_view.pop();
}
//Load Identity
void loadIdentity(){
  model_view.push(Matrix4f::Identity());
}
//Scale
void scalef(float sx, float sy, float sz){
  Matrix4f scale;
  scale<<sx,0,0,0,0,sy,0,0,0,0,sz,0,0,0,0,1;
  model_view.top() *= scale;
}
//Translate
void translatef(float x, float y, float z){
  Matrix4f translation;
  translation<<1.f,0.f,0.f,x,0.f,1.f,0.f,y,0.f,0.f,1.f,y,0.f,0.f,0.f,1.f;
  model_view.top() *= translation;
}
//Rotate, angle in degrees
void rotatef(float angle, float x, float y, float z){
  angle *=  3.1415926/360.f;
  Vector4f r(cos(angle),x,y,z);
  r.normalize();
  float a2 = r[0]*r[0],b2 = r[1]*r[1],c2 = r[2]*r[2],d2 = r[3]*r[3];
  float ab = 2*r[0]*r[1],ac = 2*r[0]*r[2],ad = 2*r[0]*r[3],bc = 2*r[1]*r[2],bd = 2*r[1]*r[3],cd = 2*r[2]*r[3];
  Matrix4f rotation;
  rotation<<a2+b2-c2-d2,bc-ad,bd+ac,0.f,bc+ad,a2+c2-b2-d2,cd-ab,0.f,bd-ac,cd+ab,a2+d2-b2-c2,0.f,0.f,0.f,0.f,1.f;
  model_view.top() *= rotation;
}
//rotate
void rotate(float rx, float ry, float rz){
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

    model_view.top() *= (rotx * roty * rotz);
}
//Scale

/* Following gluLookAt implementation is adapted from the
 * Mesa 3D Graphics library. http://www.mesa3d.org
 */
void gluLookAt(float eyex, float eyey, float eyez,
	              float centerx, float centery, float centerz,
	              float upx, float upy, float upz)
{
    Matrix4f M;
    float x[3], y[3], z[3];
    float mag;

    /* Make rotation matrix */

    /* Z vector */
    z[0] = eyex - centerx;
    z[1] = eyey - centery;
    z[2] = eyez - centerz;
    mag = (float)sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
    if (mag) {			/* mpichler, 19950515 */
        z[0] /= mag;
        z[1] /= mag;
        z[2] /= mag;
    }

    /* Y vector */
    y[0] = upx;
    y[1] = upy;
    y[2] = upz;

    /* X vector = Y cross Z */
    x[0] = y[1] * z[2] - y[2] * z[1];
    x[1] = -y[0] * z[2] + y[2] * z[0];
    x[2] = y[0] * z[1] - y[1] * z[0];

    /* Recompute Y = Z cross X */
    y[0] = z[1] * x[2] - z[2] * x[1];
    y[1] = -z[0] * x[2] + z[2] * x[0];
    y[2] = z[0] * x[1] - z[1] * x[0];

    /* mpichler, 19950515 */
    /* cross product gives area of parallelogram, which is < 1.0 for
     * non-perpendicular unit-length vectors; so normalize x, y here
     */

    mag = (float)sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
    if (mag) {
        x[0] /= mag;
        x[1] /= mag;
        x[2] /= mag;
    }

    mag = (float)sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
    if (mag) {
        y[0] /= mag;
        y[1] /= mag;
        y[2] /= mag;
    }

    M(0, 0) = x[0];
    M(0, 1) = x[1];
    M(0, 2) = x[2];
    M(0, 3) = 0.0;
    M(1, 0) = y[0];
    M(1, 1) = y[1];
    M(1, 2) = y[2];
    M(1, 3) = 0.0;
    M(2, 0) = z[0];
    M(2, 1) = z[1];
    M(2, 2) = z[2];
    M(2, 3) = 0.0;
    M(3, 0) = 0.0;
    M(3, 1) = 0.0;
    M(3, 2) = 0.0;
    M(3, 3) = 1.0;
/*    {
        int a;
        GLfixed fixedM[16];
        for (a = 0; a < 16; ++a)
            fixedM[a] = (GLfixed)(m[a] * 65536);
        glMultMatrixx(fixedM);
    }
*/
    model_view.top() *= M;
    /* Translate Eye to Origin */
   translatef(-eyex,-eyey,-eyez);

}

#endif /* defined(__nativeGraphics__model_view__) */
