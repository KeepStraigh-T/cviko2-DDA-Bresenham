#pragma once

#include <cinttypes>
#include "Mesh.h"
#include "Camera.h"

class Scene3D
{
public:
	Mesh objMesh; // Wireframe mesh
	Camera cam;
	std::vector<Vertex> sceneVertices;
	std::vector<double> zBuffer;

	void transformToCameraSpace();
	void project(int projectionType, int distance = 0);
	void initZBuffer(int64_t imgSize);
	bool isInViewVolume(idx_t currentFace, int projectionType);

	const std::vector<QColor>& getFacesColors() { return objMesh.getFacesColors(); };
	const std::vector<Vertex>& getSceneVertices() { return sceneVertices; };
	std::vector<double>* getZBuffer() { return &zBuffer; };
};

