#include "Camera.h"

Camera::Camera()
{
	viewVolume.x.min = -500.0;
	viewVolume.x.max= 500.0;
	viewVolume.y.min = -500.0;
	viewVolume.y.max = 500.0;
	viewVolume.z.min = 200.0;
	viewVolume.z.max = 200.0;
}

void Camera::setCamera(double zenit, double azimut, int distance)
{
	position.x = 0.0;
	position.y = 0.0;
	position.z = (double)distance;

	// convert angles to radians
	zenit = (pi / 180.0) * zenit;
	azimut = (pi / 180.0) * azimut;

	basisVectors.resize(3);
	basisVectors[0] = Vertex(
														sin(zenit) * sin(azimut), 
														sin(zenit) * cos(azimut),
														cos(zenit)
													);									// basis vector n - the direction the camera faces

	basisVectors[1] = Vertex(
														sin(zenit + pi / 2.0) * sin(azimut),
														sin(zenit + pi / 2.0) * cos(azimut),
														cos(zenit + pi / 2.0)
													);											// basis vector u - the twist of the camera

	basisVectors[2] = Vertex(
														basisVectors[0].y * basisVectors[1].z - basisVectors[0].z * basisVectors[1].y,
														basisVectors[0].z * basisVectors[1].x - basisVectors[0].x * basisVectors[1].z,
														basisVectors[0].x * basisVectors[1].y - basisVectors[0].y * basisVectors[1].x
													);										// basis vector v

}