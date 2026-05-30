#pragma once

#include <vector>
#include <array>
#include <cmath>
#include <numbers>
#include <iterator>
#include <cassert>

using idx_t = int;

const double pi = std::numbers::pi_v<double>; // from C++20
const double two_pi = 2.0 * pi;

class Vertex
{
public:
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;

	Vertex() {};
	Vertex(double a, double b, double c) : x(a), y(b), z(c){ }

	double dotProduct(const Vertex& other) const { return x * other.x + y * other.y + z * other.z; }
	double operator*(const Vertex& other) const { return dotProduct(other); }
};

// Wireframe 
class Mesh
{
private:
	std::vector<Vertex> vertices;
	std::vector<std::array<idx_t, 3>> faces; // triangles

public:
	Mesh();
	void buildCubeMesh(double edgeLen);
	void buildUVSphereMesh(double radius, int theta_count, int phi_count);
	void clearMesh();

	const std::vector<Vertex>& getVertices() { return vertices; };
	const std::vector<std::array<idx_t, 3>>& getFaces() { return faces; };

	void setVertices(const std::vector<Vertex>& meshVertices) { vertices = meshVertices; };
	void setFaces(const std::vector<std::array<idx_t, 3>>& meshFaces) { faces = meshFaces; };
};

