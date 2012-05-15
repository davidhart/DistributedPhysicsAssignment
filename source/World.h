#pragma once

#include <vector>
#include "ShapeBatch.h"
#include "PhysicsObjects.h"
#include "Threading.h"
#include "Vector.h"
#include "ShapeBatch.h"
#include "AABB.h"

enum eColorMode
{
	COLOR_OWNERSHIP,
	COLOR_MASS,
	COLOR_MOTION,
	COLOR_PROPERTY,
};

class World
{
	typedef std::vector<unsigned> Bucket;

public:

	World();
	~World();

	void Create(const Renderer* renderer, const Vector2d& worldMin, const Vector2d& worldMax);
	void Dispose();

	// May be called before integration and after narrowphase collision detection
	// Should not be called from multiple threads
	Physics::TriangleObject* AddTriangle();
	Physics::BoxObject* AddBox();
	Physics::BlobbyObject* AddBlobbyObject();
	Physics::BlobbyPart* AddBlobbyPart();

	void ClearObjects();
	Physics::PhysicsObject* GetObject(int id);

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

	void DetectCollisions(int bucketXMin, int bucketXMax);
	void SolveCollisions(int bucketXMin, int bucketXMax);

	void HandleUserInteraction();
	void UpdateMouseInput(const Vector2d& cursor, bool leftButton, bool rightButton);

	const std::vector<unsigned>& GetObjectsInBucket(int x, int y);

	void SetColorMode(eColorMode mode);
	eColorMode GetColorMode();

	const Vector2d& GetWorldMin();
	const Vector2d& GetWorldMax();

	int CreateTriangle();
	int CreateQuad();

	Color GetObjectColor(Physics::PhysicsObject& object);

	Physics::PhysicsObject* GetSelectedObject();

	void ResetBlobbyPressed();

	void SetOtherPeerId(int id);


	// Threadsafe calls
	void SetClientBounds(const AABB& bounds);
	void GetClientBounds(AABB& bounds);
	void SetPeerBounds(const AABB& bounds);
	void GetPeerBounds(AABB& bounds);

	void SetGravity(double gravity);
	double GetGravity();

	void SetFriction(double friction);
	double GetFriction();

	void SetElasticity(double elasticity);
	double GetElasticity();

	void SetSimSpeed(double speed);
	double GetSimSpeed();

private:

	void AddObject(Physics::PhysicsObject* object);

	void TestObjectsAgainstBucket(Bucket& objects, const Vector2i& bucket);
	void DetectCollisionsInBucket(const Vector2i& bucket);
	void SolveCollisionsInBucket(const Vector2i& bucket);

	Physics::PhysicsObject* FindObjectAtPoint(const Vector2d& point);
	Physics::PhysicsObject* FindObjectAtPointInBucket(const Vector2d& point, const Vector2i& bucketCoord);

	Vector2i GetBucketForPoint(const Vector2d& point) const;
	Vector2d GetBucketMin(int x, int y) const;
	int GetBucketIndex(const Vector2i& bucket) const;

	// Thread safe
	void SwapDrawState();

	const Quad* GetQuadDrawBuffer() const;
	const Triangle* GetTriangleDrawBuffer() const;
	
	 // Sould be called from render thread only
	void UpdatePeerBoundaryLines();
	void UpdateSpringLine();

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
	std::vector<Line> _peerBoundaryLines;

	static const int NUM_STATE_BUFFERS = 3;
	ShapeBuffer _buffers[3];

	std::vector<Physics::PhysicsObject*> _objects;

	Threading::Mutex _stateChangeMutex;
	Threading::Mutex _userInteractionMutex;
	Threading::Mutex _boundsChangeMutex;
	Threading::Mutex _springChangeMutex;

	Vector2d _springEnd;

	Vector2d _cursor;
	bool _leftButton;
	bool _rightButton;

	ShapeBatch _shapeBatch;
	LineArray _worldBoundaryBuffer;
	LineArray _peerBoundaryBuffer;
	LineArray _springBuffer;
	QuadArray _quadBuffer;
	TriangleArray _triangleBuffer;

	std::vector< Bucket > _objectBuckets;

	Vector2d _worldMin;
	Vector2d _worldMax;
	Vector2d _bucketSize;

	Physics::FixedEndSpringConstraint _cursorSpring;
	Physics::PhysicsObject* _objectTiedToCursor;

	eColorMode _colorMode;

	bool _resetBlobbyPressed;
	Physics::PhysicsObject* _blobby;

	AABB _peerBounds;
	AABB _clientBounds;
	bool _peerBoundsChanged;

	int _otherPeerId;

	static Color PEER0_COLOR;
	static Color PEER1_COLOR;

	double _gravity;
	double _elasticity;
	double _friction;

	double _simSpeed;
};