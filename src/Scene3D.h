#pragma once

#include "Mesh.h"
#include "Camera.h"

class Scene3D
{
public:
	Mesh objMesh; // Wireframe mesh
	Camera cam;
	std::vector<Vertex> sceneVertices;

	void transformToCameraSpace(); 	// project vertices from 3D object local space to Camera space
	void project(int projectionType, int distance = 0);
};

