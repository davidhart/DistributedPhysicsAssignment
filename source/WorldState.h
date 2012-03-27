#pragma once

#include <vector>
#include "ShapeBatch.h"
#include "PhysicsObjects.h"
#include "Threading.h"

class WorldState
{

public:

	WorldState();
	~WorldState();

	// Non thread safe calls
	Physics::TriangleObject* AddTriangle();
	Physics::BoxObject* AddBox();

	void UpdateTriangle(int id, const Triangle& triangle);
	void UpdateQuad(int id, const Quad& quad);
	
	const Quad* GetQuadDrawBuffer() const;
	const Triangle* GetTriangleDrawBuffer() const;

	int GetNumQuads() const;
	int GetNumTriangles() const;

	// Thread safe calls
	void SwapDrawState();
	void SwapWriteState();

	void UpdateObject(int object, double delta);
	int GetNumObjects() const;

private:

	int _writeBuffer;
	int _readBuffer;
	int _freeBuffer;
	int _drawBuffer;

	struct ShapeBuffer
	{
		std::vector<Quad> _quads;
		std::vector<Triangle> _triangles;
	};

	static const int NUM_STATE_BUFFERS = 3;
	ShapeBuffer _buffers[3];

	std::vector<Physics::PhysicsObject*> _objects;

	Threading::Mutex _stateChangeMutex;
};