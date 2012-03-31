#pragma once

#include <vector>
#include "ShapeBatch.h"
#include "PhysicsObjects.h"
#include "Threading.h"
#include "ShapeBatch.h"

class World
{

public:

	World();
	~World();

	void Create(const Renderer* renderer);
	void Dispose();

	// Non thread safe calls
	Physics::TriangleObject* AddTriangle();
	Physics::BoxObject* AddBox();

	void UpdateTriangle(int id, const Triangle& triangle);
	void UpdateQuad(int id, const Quad& quad);

	int GetNumQuads() const;
	int GetNumTriangles() const;
	
	void Draw();
	void SwapWriteState(); // Thread safe

	void UpdateObject(int object, double delta);
	int GetNumObjects() const;

private:

	void SwapDrawState(); // Thread safe

	const Quad* GetQuadDrawBuffer() const;
	const Triangle* GetTriangleDrawBuffer() const;

	int _writeBuffer;
	int _readBuffer;
	int _freeBuffer;
	int _drawBuffer;

	struct ShapeBuffer
	{
		std::vector<Quad> _quads;
		std::vector<Triangle> _triangles;
	};

	std::vector<Line> _worldBoundaryLines;

	static const int NUM_STATE_BUFFERS = 3;
	ShapeBuffer _buffers[3];

	std::vector<Physics::PhysicsObject*> _objects;

	Threading::Mutex _stateChangeMutex;

	ShapeBatch _shapeBatch;
	LineArray _worldBoundaryBuffer;
	QuadArray _quadBuffer;
	TriangleArray _triangleBuffer;
};