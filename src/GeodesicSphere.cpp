#include "GeodesicSphere.h"
#include "raymath.h"
#include <cstring>

	Mesh GeodesicSphere::createGeodesicPolyhedronMesh(int level) {

		// Definition of a normalized icosahedron
		constexpr float X = 0.525731112119133606f;
		constexpr float Z = 0.850650808352039932f;
		constexpr float N = 0.0f;

		static constexpr float initialVertices[][3] = {
			{-X, N, Z}, {X, N, Z}, {-X, N, -Z}, {X, N, -Z},
			{N, Z, X}, {N, Z, -X}, {N, -Z, X}, {N, -Z, -X},
			{Z, X, N}, {-Z, X, N}, {Z, -X, N}, {-Z, -X, N}
		};

		static constexpr unsigned short triangles[][3] = {
			{1, 4, 0}, {4, 9, 0}, {4, 5, 9}, {8, 5, 4}, {1, 8, 4},
			{1, 10, 8}, {10, 3, 8}, {8, 3, 5}, {3, 2, 5}, {3, 7, 2},
			{3, 10, 7}, {10, 6, 7}, {6, 11, 7}, {6, 0, 11}, {6, 1, 0},
			{10, 1, 6}, {11, 0, 9}, {2, 11, 9}, {5, 2, 9}, {11, 2, 7}
		};
		//////////////////////////////////////////

		// Create an initial icosahedron mesh
		Mesh icosahedron = { 0 };
		icosahedron.vertexCount = 12;
		icosahedron.triangleCount = 20;
		icosahedron.vertices = (float*)initialVertices;
		icosahedron.indices = (unsigned short*)triangles;

		// Create a new mesh from the initial icosahedron by triangulation
		// up to a given level
		Mesh currentPolyhedron = icosahedron;
		for (int i = 0; i < level; i++) {
			Mesh nextPolyhedron = divideTriangles(currentPolyhedron);
			currentPolyhedron = nextPolyhedron;
		}

		// Project all vertices on a sphere
		for (int i = 0; i < currentPolyhedron.vertexCount; i++) {
			Vector3 v = Vector3Normalize(((Vector3*)currentPolyhedron.vertices)[i]);
			((Vector3*)currentPolyhedron.vertices)[i] = v;
		}

		UploadMesh(&currentPolyhedron, false);
		return currentPolyhedron;
	}


	// Helper function
	unsigned short GeodesicSphere::findOrCreateVertexForEdge(unsigned short v1, unsigned short v2, Triangle* map, int* mapLength, Vertex* vertices, int* verticesLength) {
		unsigned short index = NULL;

		// Ensure that the edge is always defined as v1-v2 where v1 index is smaller than v2
		// in order to make finding a vertex in the mapping table easier
		if (v1 > v2) {
			unsigned short t = v1;
			v1 = v2;
			v2 = t;
		}

		// Find out if the vertex has already been created
		bool vertexFound = false;
		for (int i = 0; i < *mapLength; i++) {
			if (map[i].v1 == v1 && map[i].v2 == v2) {
				vertexFound = true;
				index = map[i].v3;
				return index;
			}
		}

		// Vertex was not found for this edge, so create a new one
		index = *verticesLength;
		vertices[index].x = (vertices[v1].x + vertices[v2].x) / 2.0f;
		vertices[index].y = (vertices[v1].y + vertices[v2].y) / 2.0f;
		vertices[index].z = (vertices[v1].z + vertices[v2].z) / 2.0f;
		(*verticesLength)++;

		map[*mapLength].v1 = v1;
		map[*mapLength].v2 = v2;
		map[*mapLength].v3 = index;
		(*mapLength)++;

		return index;
	}


	// Helper function to add a triangle to the table
	void GeodesicSphere::addTriangle(unsigned short v1, unsigned short v2, unsigned short v3, Triangle* table, int* tableLength) {
		table[*tableLength].v1 = v1;
		table[*tableLength].v2 = v2;
		table[*tableLength].v3 = v3;
		(*tableLength)++;
	}


	Mesh GeodesicSphere::divideTriangles(Mesh inputMesh) {

		// Create the output mesh structure
		Mesh outputMesh = { 0 };
		outputMesh.vertexCount = 4 * (inputMesh.vertexCount - 2) + 2;
		outputMesh.triangleCount = 4 * inputMesh.triangleCount;
		outputMesh.vertices = (float*)MemAlloc(outputMesh.vertexCount * sizeof(Vertex));
		outputMesh.indices = (unsigned short*)MemAlloc(outputMesh.triangleCount * sizeof(Triangle));

		// Initialize the output vertex table by copying all vertices from the input mesh
		memcpy(outputMesh.vertices, inputMesh.vertices, inputMesh.vertexCount * sizeof(Vertex));
		int currentVertexTableLength = inputMesh.vertexCount;

		// Output triangle table is initially empty
		int currentTriangleTableLength = 0;

		// Create a temporary table to store indices to new vertices mapped to the edges
		// for which they where created. This is to be able to find out whether a vertex
		// for a particular edge has already been created, and not duplicate it.
		Triangle* newVerticesAndEdgesMapping = (Triangle*)MemAlloc((outputMesh.vertexCount - inputMesh.vertexCount) * sizeof(Triangle));
		int currentVerticesAndEdgesMappingTableLength = 0;

		// For every triangle in the input mesh - divide it into four smaller ones
		// and add them to the output triangle table, adding also newly created vertices
		// to the output vertex table
		for (int i_triangle = 0; i_triangle < inputMesh.triangleCount; i_triangle++) {

			// Create a temporary table to store old and new vertices for the triangle being divided
			unsigned short tempIndices[6] = { 0 };

			// First three indices come from the original triangle
			Triangle* tempTriangle = (Triangle*)inputMesh.indices + i_triangle;
			tempIndices[0] = tempTriangle->v1;
			tempIndices[1] = tempTriangle->v2;
			tempIndices[2] = tempTriangle->v3;

			// Next three vertices are either created in the middle of the edges of the original triangle
			// or pulled in from a table of vertices already created for other triangles
			tempIndices[3] = findOrCreateVertexForEdge(tempTriangle->v1, tempTriangle->v2, newVerticesAndEdgesMapping, &currentVerticesAndEdgesMappingTableLength, (Vertex*)outputMesh.vertices, &currentVertexTableLength);
			tempIndices[4] = findOrCreateVertexForEdge(tempTriangle->v2, tempTriangle->v3, newVerticesAndEdgesMapping, &currentVerticesAndEdgesMappingTableLength, (Vertex*)outputMesh.vertices, &currentVertexTableLength);
			tempIndices[5] = findOrCreateVertexForEdge(tempTriangle->v3, tempTriangle->v1, newVerticesAndEdgesMapping, &currentVerticesAndEdgesMappingTableLength, (Vertex*)outputMesh.vertices, &currentVertexTableLength);

			// Add four newly created triangles to the output triangles table
			addTriangle(tempIndices[0], tempIndices[3], tempIndices[5], (Triangle*)outputMesh.indices, &currentTriangleTableLength);
			addTriangle(tempIndices[1], tempIndices[4], tempIndices[3], (Triangle*)outputMesh.indices, &currentTriangleTableLength);
			addTriangle(tempIndices[2], tempIndices[5], tempIndices[4], (Triangle*)outputMesh.indices, &currentTriangleTableLength);
			addTriangle(tempIndices[3], tempIndices[4], tempIndices[5], (Triangle*)outputMesh.indices, &currentTriangleTableLength);
		}

		return outputMesh;
	}


	GeodesicSphere::GeodesicSphere(int level) {
		mesh = createGeodesicPolyhedronMesh(level);
		model = LoadModelFromMesh(mesh);
	}

	GeodesicSphere::~GeodesicSphere() {
		UnloadMesh(mesh);
	}