#pragma once

#include <cinttypes>
#include <QDebug>
#include <algorithm>
#include "Mesh.h"
#include "Camera.h"

using std::vector;
using std::array;

class Scene3D
{
public:
	Mesh objMesh; // Wireframe mesh
	Camera cam;

private:
	QColor objColor;
	Point3D lightSource  = {0.0, 20.0, 50.0}; // light moves with camera (not in a static position)
	vector<Point3D> sceneVertices;
	vector<double> zBuffer;
	QColor lightColor {128, 128, 128};
	QColor ambientColor {128, 128, 128};
	Point3D specularCoeffs;
	Point3D diffuseCoeffs;
	Point3D lightIntensity;
	Point3D ambientIntensity;
	int sharpness;
	
public:
	void transformToCameraSpace();
	void project(int projectionType, int distance);
	void initZBuffer(int64_t imgSize);
	//bool isInViewVolume(idx_t currentFace, int projectionType);
	QColor calculateLighing(idx_t faceIdx);
	
	const std::vector<Point3D>& getVertices() { return objMesh.getVertices(); };
	std::vector<QColor>& getFacesColors() { return objMesh.getFacesColors(); };
	const std::vector<Point3D>& getSceneVertices() { return sceneVertices; };
	const QColor& getObjColor() { return objColor; };

	void setObjColor(const QColor& newColor) { objColor = newColor; }
	void setLightColor(QColor light) { lightColor = light; }
	void setAmbientColor(QColor ambient) { ambientColor = ambient; };
	void setSpecCoeffs(Point3D specCoffs) { specularCoeffs = specCoffs; };
	void setDiffCoeffs(Point3D diffCoeff) { diffuseCoeffs = diffCoeff; };
	void setLightIntensity(Point3D intensity) { lightIntensity = intensity; };
	void setAmbientIntensity(Point3D intensity) { ambientIntensity = intensity; };
	void setSharpness(int s) { sharpness = s; };
	std::vector<double>* getZBuffer() { return &zBuffer; };
};

