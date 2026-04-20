#pragma once
#include "raylib.h"


class GeodesicSphere {

	typedef struct {
		float x;
		float y;
		float z;
	} Vertex;

	typedef struct {
		unsigned short v1;
		unsigned short v2;
		unsigned short v3;
	} Triangle;

public:
	Mesh mesh = {};
	Model model = {};
	Vector3 position = { 0.0f, 0.0f, 0.0f };


	Mesh createGeodesicPolyhedronMesh(int level);
	unsigned short findOrCreateVertexForEdge(unsigned short v1, unsigned short v2, Triangle* map, int* mapLength, Vertex* vertices, int* verticesLength);
	void addTriangle(unsigned short v1, unsigned short v2, unsigned short v3, Triangle* table, int* tableLength);
	Mesh divideTriangles(Mesh inputMesh);

	GeodesicSphere(int level);
	~GeodesicSphere();
};