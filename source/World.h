#pragma once

#include <vector>
#include "ShapeBatch.h"
#include "PhysicsObjects.h"
#include "Threading.h"
#include "Vector.h"
#include "ShapeBatch.h"

class World
{
	typedef std::vector<Physics::PhysicsObject*> Bucket;

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

	// Multiple threads should not try to update the same object
	void UpdateObject(int object, double delta);
	int GetNumObjects() const;

	// Multiple threads should not try to update the same vertical strips of buckets
	void BroadPhase(int bucketXMin, int bucketXMax);
	int GetNumBucketsWide() const;
	int GetNumBucketsTall() const;

	int GetNumObjectsInBucket(int x, int y)
	{
		return _objectBuckets[GetBucketIndex(Vector2i(x, y))].size();
	}

	void SolveCollisions(int bucketXMin, int bucketXMax);

	void HandleUserInteraction();
	void UpdateMouseInput(const Vector2d& cursor, bool leftButton, bool rightButton);

private:

	void TestObjectsAgainstBucket(Bucket& objects, const Vector2i& bucket);
	void SolveCollisionsInBucket(const Vector2i& bucket);

	Physics::PhysicsObject* FindObjectAtPoint(const Vector2d& point);
	Physics::PhysicsObject* FindObjectAtPointInBucket(const Vector2d& point, const Vector2i& bucketCoord);

	Vector2i GetBucketForPoint(const Vector2d& point) const;
	Vector2d GetBucketMin(int x, int y) const;
	int GetBucketIndex(const Vector2i& bucket) const;

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
	Threading::Mutex _userInteractionMutex;

	Vector2d _cursor;
	bool _leftButton;
	bool _rightButton;

	ShapeBatch _shapeBatch;
	LineArray _worldBoundaryBuffer;
	QuadArray _quadBuffer;
	TriangleArray _triangleBuffer;

	std::vector< Bucket > _objectBuckets;

	Vector2d _worldMin;
	Vector2d _worldMax;
	Vector2d _bucketSize;

	Physics::FixedEndSpringConstraint _cursorSpring;
	Physics::PhysicsObject* _objectTiedToCursor;
};