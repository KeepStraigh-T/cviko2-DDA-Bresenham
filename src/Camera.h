#pragma once

#include "Camera.h"
#include "Mesh.h"

struct Range
{
	double min;
	double max;
};

struct Range3D
{
	Range x;
	Range y;
	Range z;

	bool inXRange(double xx) { return xx >= x.min && xx <= x.max; };
	bool inYRange(double yy) { return yy >= y.min && yy <= y.max; };
	bool inZRange(double zz) { return zz >= z.min && zz <= z.max; };
};

class Camera
{
private:
	Vertex position;
	std::vector<Vertex> basisVectors;
	Range3D viewVolume;

public:
	Camera();
	void setCamera(double zenit, double azimut, int distance);

	const std::vector<Vertex>& getBasisVectors() { return basisVectors; };
	const Vertex& getPosition() { return position; };


	const Range3D& getViewRange() const { return viewVolume; };
};