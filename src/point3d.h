#pragma once

#include <qcolor.h>

class Point3D
{
public:
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;
	QColor color = {0,0,0};

	Point3D() {}
	Point3D(double a, double b, double c) : x(a), y(b), z(c) { }

	double length() { return std::sqrt(x*x + y*y + z*z); }
	double dotProduct(const Point3D& other) const { return x * other.x + y * other.y + z * other.z; }
	Point3D crossProduct(const Point3D& other) const { return Point3D(y * other.z - z * other.y, z * other.x - x * other.z, x * other.y - y * other.x); }
	void normalize();
	double operator*(const Point3D& other) const { return dotProduct(other); }
	Point3D operator*(const double& other) const { return Point3D(x * other, y * other, z * other); } // multiplied by a scalar
	Point3D operator+(const Point3D& other) const;
	Point3D operator-(const Point3D& other) const;
};

