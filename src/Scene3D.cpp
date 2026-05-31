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
			// leave z-coordinate untouched for Z-buffer
		}
	}
}

void Scene3D::initZBuffer(int64_t imgSize)
{
	if(!zBuffer.empty())
		zBuffer.clear();

	zBuffer.resize(imgSize);
	for(int i = 0; i < imgSize; i++)
		zBuffer[i] = std::numeric_limits<double>::lowest();
}

bool Scene3D::isInViewVolume(idx_t currentFace, int projectionType)
{
	const Range3D& range = cam.getViewRange();

	const std::array<idx_t, 3>* f = objMesh.face(currentFace);
	if(!f) // no such face
		return false;

	for(size_t i = 0; i < f->size(); i++) // iterate through each vertex in given face
	{
		Vertex v = sceneVertices[f->at(i)];

		if(projectionType == 0) // parallel
		{
			bool xInside = (v.x >= range.x.min && v.x <= range.x.max) ? true : false;
			bool yInside = (v.y >= range.y.min && v.y <= range.y.max) ? true : false;
			bool zInside = (v.z >= range.z.min && v.z <= range.z.max) ? true : false;
			return (xInside || yInside || zInside);
		}
		else if(projectionType == 1) // central
		{
			double a = (pi/180.0) * 40.0; // angle (aka field of view)
			double yMax = range.z.max * std::tan(a/2.0);
			double xMax = yMax; // assume screen is a perferct square

			bool xInside = (v.x >= -xMax && v.x <= xMax) ? true : false;
			bool yInside = (v.y >= -yMax && v.y <= yMax) ? true : false;
			bool zInside = (v.z >= range.z.min && v.z <= range.z.max) ? true : false;
			return (xInside || yInside || zInside);
		}
	}
}