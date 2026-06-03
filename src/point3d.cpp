#include "point3d.h"

void Point3D::normalize()
{
	double len = length();
	if(len > 0.00001)
	{
		x = x/len; y = y/len; z = z/len;
	}
}

Point3D Point3D::operator+(const Point3D& other) const
{
	return Point3D(x + other.x, y + other.y, z + other.z);
}

Point3D Point3D::operator-(const Point3D& other) const
{
	return Point3D(x - other.x, y - other.y, z - other.z);
}