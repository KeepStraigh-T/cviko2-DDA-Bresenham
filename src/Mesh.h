#pragma once

#include <vector>
#include <array>

using idx_t = int;

struct Vertex
{
	double x = -1, y = -1, z = -1;
	Vertex() {};
	Vertex(double xx, double yy, double zz)	{ x = xx; y = yy; z = zz; };
};

class Mesh
{
private:
	std::vector<Vertex> vertices;
	std::vector<std::array<idx_t, 3>> faces; // triangles
	//std::vector<idx_t> indices; 

public:
	Mesh();
	void buildCubeMesh(double edgeLen);
	void buildUVSphereMesh(float radius, int theta_count, int phi_count);
	void clearMesh();

	const std::vector<Vertex>& getVertices() { return vertices; };
	const std::vector<std::array<idx_t, 3>>& getFaces() { return faces; };

	void setVertices(const std::vector<Vertex>& meshVertices) { vertices = meshVertices; };
	void setFaces(const std::vector<std::array<idx_t, 3>>& meshFaces) { faces = meshFaces; };
};

