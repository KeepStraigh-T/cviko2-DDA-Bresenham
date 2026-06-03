#pragma once

#include "Camera.h"
#include "Mesh.h"

//struct Range
//{
//	double min;
//	double max;
//};

//struct Range3D
//{
//	Range x;
//	Range y;
//	Range z;
//
//	bool inXRange(double xx) { return xx >= x.min && xx <= x.max; };
//	bool inYRange(double yy) { return yy >= y.min && yy <= y.max; };
//	bool inZRange(double zz) { return zz >= z.min && zz <= z.max; };
//};

class Camera
{
private:
	Point3D position;
	std::vector<Point3D> basisVectors;
	//Range3D viewVolume;
	
public:
	Camera();
	void setCamera(double zenit, double azimut, int distance);
	const std::vector<Point3D>& getBasisVectors() { return basisVectors; };
	const Point3D& getPosition() { return position; };
	//const Range3D& getViewRange() const { return viewVolume; };
};