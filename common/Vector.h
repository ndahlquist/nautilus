#pragma once
#include <cmath>
class Vector
{
public:
	float x;
	float y;
	float z;
	Vector()
	{
		x = 0;
		y = 0;
		z = 0;
	}
	Vector(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;
	}
	void set(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z; 
	}
	Vector operator - (const Vector v)
	{
		return Vector(x - v.x, y - v.y, z - v.z);
	}
	Vector operator + (const Vector v)
	{
		return Vector(x + v.x, y + v.y, z + v.z);
	}
	Vector &operator += (const Vector v)
	{
		x += v.x; 
		y += v.y; 
		z += v.z;
		return *this;
	}
	Vector &operator -= (const Vector v)
	{
		x -= v.x; 
		y -= v.y; 
		z -= v.z;
		return *this;
	}
	Vector operator * (const float f)
	{
		return Vector(x * f, y * f, z * f);
	}
	Vector operator / (const float f)
	{
		return Vector(x / f, y / f, z / f);
	}
	Vector &operator *= (const float f)
	{
		x *= f; 
		y *= f; 
		z *= f;
		return *this;
	}
	Vector &operator /= (const float f)
	{
		x /= f; 
		y /= f; 
		z /= f;
		return *this;
	}
	float length()
	{
		return sqrt(x*x+y*y+z*z);
	}
};