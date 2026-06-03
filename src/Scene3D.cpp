#include "Scene3D.h"

void Scene3D::transformToCameraSpace()
{
	const std::vector<Point3D>& meshVertices = objMesh.getVertices();
	const std::vector<Point3D>& basisVectores = cam.getBasisVectors();
	Point3D position = cam.getPosition();

	if(meshVertices.empty() || basisVectores.empty())
		return;

	sceneVertices.resize(meshVertices.size());
	// transform to camera space
	for(size_t i = 0; i < meshVertices.size(); i++)
		sceneVertices[i] = Point3D(meshVertices[i] * basisVectores[2], meshVertices[i] * basisVectores[1], meshVertices[i] * basisVectores[0]); // dot product
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

QColor Scene3D::calculateLighing(idx_t faceIdx)
{
	const std::array<idx_t, 3>* f = objMesh.face(faceIdx);
	if(!f)
		return QColor(0, 0, 0);

	// 1. Compute normal to the face
	Point3D V0 = sceneVertices[f->at(0)];
	Point3D V1 = sceneVertices[f->at(1)];
	Point3D V2 = sceneVertices[f->at(2)];
	Point3D U = V1 - V0;
	Point3D M = V2 - V0;

	Point3D N = U.crossProduct(M);
	N.normalize();

	// 2. Compute direction of the light source
	// for flat shading use geometric center of the face
	Point3D S((V0.x + V1.x + V2.x) / 3.0, (V0.y + V1.y + V2.y) / 3.0, (V0.z + V1.z + V2.z) / 3.0);
	Point3D L = lightSource - S;
	L.normalize();

	// 3. Compute direction of the reflection
	Point3D R = N * ((N*L) * 2.0) - L;
	R.normalize();
	// 4 Compute direction of the camera
	Point3D V(-S.x, -S.y, -S.z); // camera is at the center of system
	V.normalize();

	// 5. Reflection component
	Point3D Ilrs(lightIntensity.x * specularCoeffs.x, lightIntensity.y * specularCoeffs.y, lightIntensity.z * specularCoeffs.z);
	Point3D Is = Ilrs * std::pow(std::max(0.0, V*R), sharpness);

	// 6. Diffusion component
	Point3D Ilrd(lightIntensity.x * diffuseCoeffs.x, lightIntensity.y * diffuseCoeffs.y, lightIntensity.z * diffuseCoeffs.z);
	Point3D Id = Ilrd * std::max(0.0, L*N);

	// 7. Ambient component
	// take same coefficient as for diffusion
	Point3D Ia(ambientIntensity.x * diffuseCoeffs.x, ambientIntensity.y * diffuseCoeffs.y, ambientIntensity.z * diffuseCoeffs.z);

	// 8 Final intensity
	Point3D I = Is + Id + Ia;
	//qDebug() << I.x << ' ' << I.y << ' ' << I.z << '\n';
	int r = std::clamp(static_cast<int>(objColor.redF() * I.x), 0, 255);
	int g = std::clamp(static_cast<int>(objColor.greenF() * I.y), 0, 255);
	int b = std::clamp(static_cast<int>(objColor.blueF() * I.z), 0, 255);
	return QColor(r, g, b);
	//return QColor(objColor.redF() * I.x, objColor.greenF() * I.y, objColor.blueF() * I.z);

}

//bool Scene3D::isInViewVolume(idx_t currentFace, int projectionType)
//{
//	const Range3D& range = cam.getViewRange();
//
//	const std::array<idx_t, 3>* f = objMesh.face(currentFace);
//	if(!f) // no such face
//		return false;
//
//	for(size_t i = 0; i < f->size(); i++) // iterate through each Point3D in given face
//	{
//		Point3D v = sceneVertices[f->at(i)];
//
//		if(projectionType == 0) // parallel
//		{
//			bool xInside = (v.x >= range.x.min && v.x <= range.x.max) ? true : false;
//			bool yInside = (v.y >= range.y.min && v.y <= range.y.max) ? true : false;
//			bool zInside = (v.z >= range.z.min && v.z <= range.z.max) ? true : false;
//			return (xInside || yInside || zInside);
//		}
//		else if(projectionType == 1) // central
//		{
//			double a = (pi/180.0) * 40.0; // angle (aka field of view)
//			double yMax = range.z.max * std::tan(a/2.0);
//			double xMax = yMax; // assume screen is a perferct square
//
//			bool xInside = (v.x >= -xMax && v.x <= xMax) ? true : false;
//			bool yInside = (v.y >= -yMax && v.y <= yMax) ? true : false;
//			bool zInside = (v.z >= range.z.min && v.z <= range.z.max) ? true : false;
//			return (xInside || yInside || zInside);
//		}
//	}
//}