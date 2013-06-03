#include <iostream>
#include <GLUT/glut.h>
#include <Eigen/Dense>
#include <cmath>
#include <OpenMesh/Core/IO/Options.hh>
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <stdlib.h>
#include <time.h>

using namespace std;
using namespace Eigen;
using namespace OpenMesh;

double numSubT = 3; //this is the number of pieces each wall segment will be broken into
Eigen::Matrix4d m;
Eigen::Matrix4d b;
// Eigen::Matrix4d firstM;
// Eigen::Matrix4d lastM;
const int TOLERANCE = 0.00001f;
const double spacing = 1;
const double pi = 3.14159265359;
const int offsetLimit = 100;
double offsetMax = (double)offsetLimit * 3;

Vec3f getPoint(double u, Vec3f p1, Vec3f p2, Vec3f p3, Vec3f p4) {
	//cout << p1 << ", " << p2 << ", " << p3 << ", " << p4 << endl;
	Vec3f point;
	Vector4d c;
	double usquared = u*u;
	double ucubed = usquared*u;	
	//calculate x
	Eigen::Vector4d xDim;
	xDim << p1[0], p2[0], p3[0], p4[0];
	c = m * xDim;
	//c /= 6;
	point[0] = c[0] + c[1]*u + c[2]*usquared + c[3]*ucubed;
	//point[0] = c[3] + c[2]*u + c[1]*usquared + c[0]*ucubed;
	//calculate y
	Eigen::Vector4d yDim;
	yDim << p1[1], p2[1], p3[1], p4[1];
	c = m * yDim;
	//c /= 6;
	point[1] = c[0] + c[1]*u + c[2]*usquared + c[3]*ucubed;
	//point[1] = c[3] + c[2]*u + c[1]*usquared + c[0]*ucubed;
	//calculate z
	Eigen::Vector4d zDim;
	zDim << p1[2], p2[2], p3[2], p4[2];
	c = m * zDim;
	//c /= 6;
	point[2] = c[0] + c[1]*u + c[2]*usquared + c[3]*ucubed;
	//point[2] = c[3] + c[2]*u + c[1]*usquared + c[0]*ucubed;
	//cout << point << endl;
	return point;
}

void drawSpline(vector<Vec3f> frames){
	if (frames.size() == 0) return;
	glColor3f(1.0, 0.0, 0.0);
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < frames.size(); i++){
		 glVertex3f(frames[i][0], frames[i][1], frames[i][2]);
	}
	glEnd();
}

vector<vector<Vec3f> > genControlPts(double width, double length, double height){
	m << 0.0, 1.0, 0.0, 0.0,
	-0.5, 0.0, 0.5, 0.0,
	1.0, -2.5, 2, -0.5,
	-0.5, 1.5, -1.5, 0.5;

	b << -1.0, 3.0, -3.0, 1.0,
	3.0, -6.0, 3.0, 0.0,
	-3.0, 0, 3.0, 0.0,
	1.0, 4.0, 1.0, 0.0;

	srand(time(NULL));
	if (width > length) {
		double temp = width;
		width = length;
		length = temp;
	}
	double a = length/2; 
	double b = width/2;
	double h = (a - b)*(a - b) / ( (a + b)*(a + b) );
	double C = pi * (a+b) * (1 + (3*h)/(10 + sqrt(4-3*h) ) );
	int numHoriz = 2 * ((C/2)/spacing); //should be even 
	double angleInc = 2*pi / (double)numHoriz;
	double vertSpacing = height / 4;
	//rand() % 10 range 0-9
	int dec = 10;
	vector<vector<Vec3f> > control; //control points
	for (int i = 3; i >= 0; i--){
		a -= (double)(rand()%dec) /40; 
		b -= (double)(rand()%dec) /40;
		h = (a - b)*(a - b) / ( (a + b)*(a + b) );
		vector<Vec3f> row; 
		for (int j = 0; j < numHoriz; j++){
			double r = (a*b)/sqrt( (b*cos(j*angleInc))*(b*cos(j*angleInc)) + (a*sin(j*angleInc))*(a*sin(j*angleInc)) );
			double x = -r*cos(j*angleInc) + (double)(rand()%(offsetLimit*2) - offsetLimit)/ offsetMax;
			double y = i*vertSpacing + (double)(rand()%(offsetLimit*2) - offsetLimit)/ (offsetMax*2);
			double z = r*sin(j*angleInc) + (double)(rand()%(offsetLimit*2) - offsetLimit)/ offsetMax;
			Vec3f point(x, y, z);
			row.push_back(point);
			//y coordinate is in some range of i*vertSpacing
		}
		control.push_back(row);
		dec += 5;
	}
	//numSubT = spacing/0.5;
	vector<vector<Vec3f> > intplt;	//interpolated points
	Vec3f iPoint;
	for (int k = 0; k < 4; k++){
    	vector<Vec3f> pts;
		for (int i = 0; i < numHoriz; i++){ //i is index of keyframe beginning segment
			double j = 0;
			//for (double j = 0; j < 1; j += 1/numSubT){
			for (int loop = 0; loop < numSubT; loop++){
				iPoint = getPoint(j, control[k][(i-1 + numHoriz)%numHoriz], control[k][i], control[k][(i+1)%numHoriz], control[k][(i+2)%numHoriz]);
				pts.push_back(iPoint);
				j += 1/numSubT;
			}
		}
		pts.push_back(pts[0]);
		intplt.push_back(pts);
	}
	return intplt;
}

vector<vector<Vec3f> > readControlPts(string filename) {
	m << 0.0, 1.0, 0.0, 0.0,
	-0.5, 0.0, 0.5, 0.0,
	1.0, -2.5, 2, -0.5,
	-0.5, 1.5, -1.5, 0.5;

	b << -1.0, 3.0, -3.0, 1.0,
	3.0, -6.0, 3.0, 0.0,
	-3.0, 0, 3.0, 0.0,
	1.0, 4.0, 1.0, 0.0;

	// firstM << 0.0, 1.0, 0.0, 0.0,
	// 0.0 , -1.0, 1.0, 0.0,
	// 0.0, -0.5, 1.0, -0.5,
	// 0.0, 0.5, -1.0, 0.5;

	// lastM << 0.0, 1.0, 0.0, 0.0,
	// -0.5, 0.0, 0.5, 0.0,
	// 1.0, -2.0, 1.0, 0.0,
	// -0.5, 1.0, -0.5, 0.0;

	vector<vector<Vec3f> > kframes;
	vector<vector<Vec3f> > iframes;
	ifstream data;
	string line;
	int perRow;
    data.open(filename.c_str());
    if (data.is_open()) {
    	getline(data, line);
    	sscanf(line.c_str(),"%d", &perRow);
        while (data.good() && !data.eof()) {
        	vector<Vec3f> pts;
        	// for (int i = 0; i < perRow; i++) {
	        // 	double x, y, z;
	        //     getline(data, line);
	        //     int c = sscanf(line.c_str(), "v%lf v%lf v%lf", &x, &y, &z);
	        //     if (c > 0){
		       //      Eigen::Vec3f ctrlPt;
		       //      ctrlPt[0] = x; ctrlPt[1] = y; ctrlPt[2] = z;
		       //      pts.push_back(ctrlPt);
	        // 	} else {
	        // 		i--;
	        // 	}
	        // }
	        while (data.good() && !data.eof()) {

	        	double x, y, z;
	            getline(data, line);
	            int c = sscanf(line.c_str(), "v%lf v%lf v%lf", &x, &y, &z);
	            if (c > 0){
		            Vec3f ctrlPt;
		            ctrlPt[0] = x; ctrlPt[1] = y; ctrlPt[2] = z;
		            pts.push_back(ctrlPt);
		            //cout << x << ", " << y << ", " << z << endl;
	        	} else {
	        		break;
	        	}
	        }
	        kframes.push_back(pts);
        }
    } else {
    	cout << "Error reading file." << endl;
    	vector<vector<Vec3f> > empty;
    	return empty;
    }

    //Do interpolation
    Vec3f iPoint;
    // bool head;
    // bool tail;
    //kframes.size() should == 4
    for (int k = 0; k < 4; k++){
    	vector<Vec3f> pts;
		for (int i = 0; i < perRow; i++){ //i is index of keyframe beginning segment
			double j = 0;
			//for (double j = 0; j < 1; j += 1/numSubT){
			for (int loop = 0; loop < numSubT; loop++){
				// (i == 0) ? head = true : head = false;//first segment
				// (i == kframes.size() - 2) ? tail = true : tail = false;//last segment
				iPoint = getPoint(j, kframes[k][(i-1 + perRow)%perRow], kframes[k][i], kframes[k][(i+1)%perRow], kframes[k][(i+2)%perRow]);
				pts.push_back(iPoint);
				j += 1/numSubT;
			}

		}
		pts.push_back(pts[0]);
		iframes.push_back(pts);
	}
    return iframes;
}