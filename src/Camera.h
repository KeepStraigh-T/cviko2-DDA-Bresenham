#pragma once

#include "Camera.h"
#include "Mesh.h"

class Camera
{
private:
	Vertex position;
	std::vector<Vertex> basisVectors;

public:
	void setCamera(double zenit, double azimut, int distance);

	const std::vector<Vertex>& getBasisVectors() { return basisVectors; };
	const Vertex& getPosition() { return position; };
};