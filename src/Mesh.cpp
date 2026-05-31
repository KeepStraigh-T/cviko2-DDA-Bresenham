#include "Mesh.h"

using namespace std;

Mesh::Mesh(){};

void Mesh::buildCubeMesh(double edgeLen)
{
	if(edgeLen == 0.0)
		return;

	clearMesh();

	double x = edgeLen, y = edgeLen, z = edgeLen;
	const int num_CubeVertices = 8;

	vertices.resize(num_CubeVertices);
	vertices[0] = Vertex( -x/2.0, -y/2.0, -z/2.0 ); // Index 0: Back-Bottom-Left
	vertices[1] = Vertex( -x/2.0, y/2.0, -z/2.0 ); // Index 1: Back-Bottom-Right
	vertices[2] = Vertex( x/2.0, y/2.0, -z/2.0 ); // Index 2: Front-Bottom-Right
	vertices[3] = Vertex( x/2.0, -y/2.0, -z/2.0 ); // Index 3: Front-Bottom-Left
	vertices[4] = Vertex( -x/2.0, -y/2.0, z/2.0 ); // Index 4: Back-Top-Left
	vertices[5] = Vertex( -x/2.0, y/2.0, z/2.0 ); // Index 5: Back-Top-Right
	vertices[6] = Vertex( x/2.0, y/2.0, z/2.0 ); // Index 6: Front-Top-Right
	vertices[7] = Vertex( x/2.0, -y/2.0, z/2.0 ); // Index 7: Front-Top-Left

	const int nFaces = 12;
	faces.resize(nFaces);
	faces[0] = {0, 1, 2};
	faces[1] = {0, 2, 3};
	faces[2] = {1, 5, 2};
	faces[3] = {2, 5, 6};
	faces[4] = {5, 4, 7};
	faces[5] = {7, 6, 5};
	faces[6] = {7, 2, 6};
	faces[7] = {7, 3, 2};
	faces[8] = {4, 0, 7};
	faces[9] = {7, 0, 3};
	faces[10] = {0, 1, 5}; 
	faces[11] = {5, 4, 0};

	facesColors.resize(12);
	facesColors[0].setRgb(128, 128, 128);
	facesColors[1].setRgb(0, 0, 0);
	facesColors[2].setRgb(0, 0, 255);
	facesColors[3].setRgb(0, 0, 255);
	facesColors[4].setRgb(0, 255, 0);
	facesColors[5].setRgb(0, 255, 0);
	facesColors[6].setRgb(255, 0, 0);
	facesColors[7].setRgb(255, 0, 0);
	facesColors[8].setRgb(255, 234, 0);
	facesColors[9].setRgb(255, 234, 0);
	facesColors[10].setRgb(255, 165, 255);
	facesColors[11].setRgb(255, 165, 255);
}

void Mesh::buildUVSphereMesh(double radius, int theta_count, int phi_count)
{
	clearMesh();

	const double theta_step = pi / theta_count; // theta step (vertical angle): {0, Pi}
	const double phi_step = two_pi / phi_count; // phi step (horizontal angle): {0, 2Pi}

	const int num_vertices = (theta_count - 1) * phi_count + 2; // +2 for top and bottom vertices
	vertices.resize((size_t)num_vertices);

	idx_t c = 0;

	// Add the top vertex
	vertices[c++] = (Vertex(0.0, radius, 0.0));

	// Generate the mid-section vertex grid
	for(int i = 1; i <= theta_count - 1; i++)
	{
		double theta = i * theta_step;
		for(int j = 0; j < phi_count; j++)
		{
			double phi = j * phi_step;
			vertices[c++] =  Vertex(
															radius*sin(theta)*cos(phi),
															radius*cos(theta),
															-radius*sin(theta)*sin(phi) // "-" to respect right-hand rule
															);
		}
	}

	// Add last bottom vertex
	vertices[c] = Vertex(0.0, -radius, 0.0);
	//assert(c == num_vertices - 1);
	 
	const int num_faces = 2*phi_count + 2*phi_count*(theta_count - 2);
	faces.resize(num_faces);
	facesColors.resize(num_faces);

	c = 0;
	// Faces on the top cap, connecting the north pole to the first ring
	for(int i = 0; i < (phi_count - 1); i++)
		faces[c++] = {0, i+1, i+2};

	faces[c++] = {0, phi_count, 1}; // add last face on the top cap

	// Faces between the top and bottom rings
	const idx_t north_pole_index = 1; 
	for(int i = 0; i < (theta_count - 2); i++)
	{
		for(int j = 0; j < phi_count; j++)
		{
			int next_j = (j+1) % phi_count; // wrap around to add last face on each parallel
			const int index[4]{
				north_pole_index + i*phi_count + j,				
				north_pole_index + (i + 1)*phi_count + j,
				north_pole_index + (i + 1)*phi_count + next_j,
				north_pole_index + i*phi_count + next_j
			};

			faces[c++] = {index[0], index[1], index[2]}; // first triangle
			faces[c++] = {index[0], index[2], index[3]}; // second triangle
			
		}
	}

	// Faces on bottom cap, connecting the south pole to the last ring
	const idx_t south_pole_index = (idx_t)vertices.size() - 1;
	for(int i = 0; i < (phi_count - 1); i++)
	{
		faces[c++] = {
			south_pole_index,
			south_pole_index - phi_count + i + 1,
			south_pole_index - phi_count + i
		};
	}

	// add last face on bottom cap
	faces[c] = {
		south_pole_index,
		south_pole_index - phi_count,
		south_pole_index - 1
	};
	//assert(c == num_faces - 1);

	for(int i = 0; i < faces.size(); i++)
		facesColors[i] = Qt::blue;

}

void Mesh::clearMesh()
{
	vertices.clear();
	faces.clear();
	facesColors.clear();
}