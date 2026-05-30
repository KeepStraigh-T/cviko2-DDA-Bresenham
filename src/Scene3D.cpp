#include "Scene3D.h"

void Scene3D::transformToCameraSpace()
{
	const std::vector<Vertex>& meshVertices = objMesh.getVertices();
	const std::vector<Vertex>& basisVectores = cam.getBasisVectors();
	Vertex position = cam.getPosition();

	if(meshVertices.empty() || basisVectores.empty())
		return;

	sceneVertices.resize(meshVertices.size());
	// transform to camera space
	for(size_t i = 0; i < meshVertices.size(); i++)
		sceneVertices[i] = Vertex(meshVertices[i] * basisVectores[2], meshVertices[i] * basisVectores[1], meshVertices[i] * basisVectores[0]); // dot product
}

void Scene3D::project(int projectionType, int distance)
{
	if(projectionType == 0) // parallel
		return;

	else if(projectionType == 1) // central
	{
		for(int i = 0; i < sceneVertices.size(); i++)
		{
			sceneVertices[i].x = distance * sceneVertices[i].x / (distance - sceneVertices[i].z);
			sceneVertices[i].y = distance * sceneVertices[i].y / (distance - sceneVertices[i].z);
			sceneVertices[i].z = 0.0;
		}
	}
}