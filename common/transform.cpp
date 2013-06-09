//  transform.cpp

#include "transform.h"

#include "Eigen/LU"

std::stack<Matrix4f> model_view;
std::stack<Matrix4f> projection;

#define PI 3.1415926536
//Model-view
//get the current matrix
float* mvMatrix(){
    float* mvMatrix = new float[16];
    for(int i=0; i<4; i++)
        for(int j=0; j<4; j++)
            mvMatrix[i*4+j] = model_view.top()(j,i);
    return mvMatrix;
}
//Push
void mvPushMatrix(){
    model_view.push(model_view.top());
}
//Pop
void mvPopMatrix(){
    model_view.pop();
}
//Load Identity
void mvLoadIdentity(){
    model_view.push(Matrix4f::Identity());
}
//Scale
void scalef(float s) {
    scalef(s, s, s);
}
void scalef(float sx, float sy, float sz){
    Matrix4f scale;
    scale<<sx,0,0,0,0,sy,0,0,0,0,sz,0,0,0,0,1;
    model_view.top() *= scale;
}
//Translate
void translate(Eigen::Vector3f translation){
    translatef(translation[0], translation[1], translation[2]);
}
void translatef(float x, float y, float z){
    Matrix4f translation;
    translation<<1.f,0.f,0.f,x,0.f,1.f,0.f,y,0.f,0.f,1.f,z,0.f,0.f,0.f,1.f;
    model_view.top() *= translation;
}
//Rotate, angle in degrees
void rotatef(float angle, float x, float y, float z){
    angle *=  PI;
    angle /= 180.f;
    float cos_theta = cosf(angle);
    float _cos_theta = 1.f - cos_theta;
    float sin_theta = sqrt(1.f-cos_theta*cos_theta);
    Vector3f u(x,y,z);
    u.normalize();
    float u_xy = u[0]*u[1]*_cos_theta,u_xz = u[0]*u[2]*_cos_theta,
    u_yz = u[1]*u[2]*_cos_theta,u_xx = u[0]*u[0]*_cos_theta,
    u_yy = u[1]*u[1]*_cos_theta,u_zz = u[2]*u[2]*_cos_theta,
    u_x = u[0]*sin_theta, u_y = u[1]*sin_theta, u_z = u[2]*sin_theta;
    Matrix4f rotation;
    rotation<<cos_theta+u_xx,u_xy-u_z,u_xz+u_y,0.f,
    u_xy+u_z,cos_theta+u_yy,u_yz-u_x,0.f,
    u_xz-u_y,u_yz+u_x,cos_theta+u_zz,0.f,
    0.f,0.f,0.f,1.f;
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
void lookAt(float eyex, float eyey, float eyez,
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
    
    mag = (float)sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);
    if (mag) {
        x[0] /= mag;
        x[1] /= mag;
        x[2] /= mag;
    }
    
    /* Recompute Y = Z cross X */
    y[0] = z[1] * x[2] - z[2] * x[1];
    y[1] = -z[0] * x[2] + z[2] * x[0];
    y[2] = z[0] * x[1] - z[1] * x[0];
    
    /* mpichler, 19950515 */
    /* cross product gives area of parallelogram, which is < 1.0 for
     * non-perpendicular unit-length vectors; so normalize x, y here
     */
    
    
    /* mag = (float)sqrt(y[0] * y[0] + y[1] * y[1] + y[2] * y[2]);
     if (mag) {
     y[0] /= mag;
     y[1] /= mag;
     y[2] /= mag;
     }*/
    
    M<<x[0],x[1],x[2],0.f,y[0],y[1],y[2],0.f,z[0],z[1],z[2],0.f,
    0.f,0.f,0.f,1.f;
    
    model_view.top() *= M;
    translatef(-eyex,-eyey,-eyez);
}
//projection
float* pMatrix(){
    float* pMatrix = new float[16];
    for(int i=0; i<4; i++)
        for(int j=0; j<4; j++)
            pMatrix[i*4+j] = projection.top()(j,i);
    return pMatrix;
}

float* pInverseMatrix(){
    Matrix4f inverse = projection.top().inverse();
    float* pMatrix = new float[16];
    for(int i=0; i<4; i++)
        for(int j=0; j<4; j++)
            pMatrix[i*4+j] = inverse(j,i);
    return pMatrix;
}

//Push
void pPushMatrix(){
    projection.push(projection.top());
}
//Pop
void pPopMatrix(){
    projection.pop();
}
//Load Identity
void pLoadIdentity(){
    projection.push(Matrix4f::Identity());
}
//frustum
void frustum(double left, double right, double bottom, double top,
             double nearVal, double farVal ){
    double r_l = right - left, t_b = top - bottom, f_n = farVal - nearVal,
    _nearVal = 2.f*nearVal;
    Matrix4f frustum;
    frustum<<_nearVal/r_l,0.f,(right+left)/r_l,0.f,
    0.f,_nearVal/t_b,(top+bottom)/t_b,0.f,
    0.f,0.f,-(farVal+nearVal)/f_n,-farVal*_nearVal/f_n,
    0.f,0.f,-1.f,0.f;
    projection.top() *= frustum;
}
//Perspective
void perspective(float fovy, float aspect,
                 float zNear, float zFar) {
    float xmin, xmax, ymin, ymax;
    
    ymax = zNear * (float)tanf(fovy * PI / 360);
    ymin = -ymax;
    xmin = ymin * aspect;
    xmax = ymax * aspect;
    
    frustum(xmin, xmax,ymin,ymax,zNear,zFar);
}
void viewport(int x, int y, int width, int height){
    float width_2 = width/2.f,height_2 = height/2.f;
    Matrix4f scale;
    scale<<width_2,0.f,0.f,0.f,0.f,height_2,0.f,0.f,0.f,0.f,0.5f,0.f,
    0.f,0.f,0.f,1.f;
    Matrix4f translate;
    translate<<1.f,0.f,0.f,x+width_2,0.f,1.f,0.f,y+height_2,0.f,0.f,1.f,0.5f,
    0.f,0.f,0.f,1.f;
    projection.top() *= (scale*translate);
}
//return combined mvp matrix
float* mvpMatrix(){
    Matrix4f mvp = projection.top()*model_view.top();
    float* mvpMatrix = new float[16];
    for(int i=0; i<4; i++)
        for(int j=0; j<4; j++)
            mvpMatrix[i*4+j] = mvp(j,i);
    return mvpMatrix;  
}
