#pragma once

#include <vector>
#include <array>
#include <cmath>
#include <iterator>
#include <cassert>
#include <qcolor.h>
#include "point3d.h"

using idx_t = int;

const double pi = 3.14159265359;
const double two_pi = 2.0 * pi;

// Wireframe 
class Mesh
{
private:
	std::vector<Point3D> vertices;
	std::vector<std::array<idx_t, 3>> faces; // triangles
	std::vector<QColor> facesColors; // cube sides colors
	//QColor spherColor;

public:
	Mesh();
	void buildCubeMesh(double edgeLen);
	void buildUVSphereMesh(double radius, int theta_count, int phi_count, QColor color);
	void clearMesh();

	const std::vector<Point3D>& getVertices()  { return vertices; };
	const Point3D* point(idx_t i)  { return i < vertices.size() ? &vertices[i] : nullptr; };
	const std::vector<std::array<idx_t, 3>>& getFaces()  { return faces; };
	const std::array<idx_t, 3>* face(idx_t i)  { return i < faces.size() ? &faces[i] : nullptr; };
	std::vector<QColor>& getFacesColors()  { return facesColors; };

	void setVertices(const std::vector<Point3D>& meshVertices) { vertices = meshVertices; };
	void setFaces(const std::vector<std::array<idx_t, 3>>& meshFaces) { faces = meshFaces; };
};

