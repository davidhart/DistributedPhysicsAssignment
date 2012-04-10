#include "World.h"

World::World() :
	_drawBuffer(0),
	_readBuffer(0),
	_writeBuffer(1),
	_freeBuffer(2),
	_worldMin(-20, 0),
	_worldMax(20, 20)
{
	_bucketSize = (_worldMax - _worldMin) / Vector2d(GetNumBucketsWide(), GetNumBucketsTall());
	_objectBuckets.resize(GetNumBucketsTall()*GetNumBucketsWide());

	/*
	for (unsigned i = 0; i < _objectBuckets.size(); ++i)
	{
		_objectBuckets[i].reserve(500);
	}*/
}

int World::GetNumBucketsTall() const
{
	return 10;
}

int World::GetNumBucketsWide() const
{
	return 20;
}

World::~World()
{
	for (unsigned i = 0; i < _objects.size(); ++i)
	{
		delete _objects[i];
	}
}

void World::Create(const Renderer* renderer)
{
	_shapeBatch.Create(renderer);

	_shapeBatch.AddArray(&_quadBuffer);
	_shapeBatch.AddArray(&_triangleBuffer);

	_worldBoundaryLines.resize(4 + (GetNumBucketsWide() - 1) + (GetNumBucketsTall() - 1));
	
	Line l;
	l._color = Color(1.0f, 0.0f, 0.0f, 1.0f);
	l._points[0] = Vector2f((float)_worldMin.x(), (float)_worldMax.y());
	l._points[1] = Vector2f((float)_worldMin.x(), (float)_worldMin.y());

	_worldBoundaryLines[0] = l;

	l._points[0].x((float)_worldMax.x());
	l._points[1].x((float)_worldMax.x());
	_worldBoundaryLines[1] = l;

	l._points[0] = Vector2f((float)_worldMin.x(), (float)_worldMax.y());
	l._points[1] = Vector2f((float)_worldMax.x(), (float)_worldMax.y());
	_worldBoundaryLines[2] = l;

	l._points[0].y((float)_worldMin.y());
	l._points[1].y((float)_worldMin.y());
	_worldBoundaryLines[3] = l;

	l._color = Color(0.5f, 0.5f, 0.5f, 1.0f);
	l._points[0].y(_worldMin.y());
	l._points[1].y(_worldMax.y());

	for (int i = 1; i < GetNumBucketsWide(); ++i)
	{
		l._points[0].x(GetBucketMin(i, 0).x());
		l._points[1].x(l._points[0].x());
		_worldBoundaryLines[4 + i - 1] = l;
	}

	l._points[0].x(_worldMin.x());
	l._points[1].x(_worldMax.x());

	for (int i = 1; i < GetNumBucketsTall(); ++i)
	{
		l._points[0].y(GetBucketMin(0, i).y());
		l._points[1].y(l._points[0].y());
		_worldBoundaryLines[4 + GetNumBucketsWide() + i - 2] = l;
	}

	_shapeBatch.AddArray(&_worldBoundaryBuffer);
	_worldBoundaryBuffer.SetShapes(&_worldBoundaryLines[0], _worldBoundaryLines.size());
}

void World::Dispose()
{
	_shapeBatch.Dispose();
}

Physics::TriangleObject* World::AddTriangle()
{
	for (int i = 0; i < NUM_STATE_BUFFERS; i++)
	{
		_buffers[i]._triangles.push_back(Triangle());
	}

	Physics::TriangleObject* triangle = new Physics::TriangleObject(_buffers[0]._triangles.size() - 1);

	_objects.push_back(triangle);

	return triangle;
}

Physics::BoxObject* World::AddBox()
{
	for (int i = 0; i < NUM_STATE_BUFFERS; i++)
	{
		_buffers[i]._quads.push_back(Quad());
	}

	Physics::BoxObject* box = new Physics::BoxObject(_buffers[0]._quads.size() - 1);

	_objects.push_back(box);

	return box;
}

void World::BroadPhase(int bucketXMin, int bucketXMax)
{
	// Clear buckets this thread manages
	Vector2i bucket;
	for (bucket.x(bucketXMin); bucket.x() <= bucketXMax; bucket.x(bucket.x() + 1))
	{
		for (bucket.y(0); bucket.y() < GetNumBucketsTall(); bucket.y(bucket.y() + 1))
		{
			int index = GetBucketIndex(bucket);
			_objectBuckets[index].clear();
		}
	}

	double bucketXMinWorldSpace = GetBucketMin(bucketXMin, 0).x();
	double bucketXMaxWorldSpace = bucketXMinWorldSpace + _bucketSize.x() * (bucketXMax - bucketXMin + 1);

	for (unsigned i = 0; i < _objects.size(); ++i)
	{
		Vector2d position = _objects[i]->GetPosition();

		// If the position is somewhere within the band this thread is concerned with
		if (   position.x() >= bucketXMinWorldSpace 
			&& position.x() <  bucketXMaxWorldSpace  )
		{
			//_objects[i]->test++;

			/*
			int test = _objects[i]->test;

			if (test > 1)
			{
				std::cout << _objects[i]->GetPosition() << std::endl;
			}*/

			Vector2i bucket = GetBucketForPoint(position);

			_objectBuckets[GetBucketIndex(bucket)].push_back(_objects[i]);
		}
	}
}

void World::SolveCollisions(int bucketXMin, int bucketXMax)
{
	/*
	for (int o = bucketXMin; o <= bucketXMax; o++)
	{
		_objects[o]->UpdateShape(*this);
	}*/
	Vector2i bucket;
	for (bucket.x(bucketXMin); bucket.x() <= bucketXMax; bucket.x(bucket.x() + 1))
	{
		for (bucket.y(0); bucket.y() < GetNumBucketsTall(); bucket.y(bucket.y() + 1))
		{
			SolveCollisionsInBucket(bucket);
		}
	}
}

void World::TestObjectsAgainstBucket(Bucket& objects, const Vector2i& bucket)
{
	if (bucket.x() < 0) return;
	if (bucket.x() >= GetNumBucketsWide()) return;
	if (bucket.y() < 0) return;
	if (bucket.y() >= GetNumBucketsTall()) return;

	Bucket& testBucket = _objectBuckets[GetBucketIndex(bucket)];

	Physics::Collision collision;

	for (unsigned i = 0; i < objects.size(); ++i)
	{
		for (unsigned j = 0; j < testBucket.size(); ++j)
		{
			if (objects[i]->TestCollision(*testBucket[j], collision))
			{
				objects[i]->CollisionResponse(collision._collisionNormal);
				objects[i]->test = 1;
			}
			
		}
	}
}

void World::SolveCollisionsInBucket(const Vector2i& bucket)
{
	Bucket& bucketObjects = _objectBuckets[GetBucketIndex(bucket)];

	Physics::Collision collision;

	for (unsigned i = 0; i < bucketObjects.size(); ++i)
	{
		for (unsigned j = 0; j < bucketObjects.size(); ++j)
		{
			if (bucketObjects[i]->TestCollision(*bucketObjects[j], collision))
			{
				bucketObjects[i]->CollisionResponse(collision._collisionNormal);
				bucketObjects[i]->test = 1;
				bucketObjects[j]->CollisionResponse(-collision._collisionNormal);
				bucketObjects[j]->test = 1;
			}
		}
	}

	for (int x = -1; x < 2; x++)
	{
		for (int y = -1; y < 2; y++)
		{
			if (!(x == 0 && y == 0))
				TestObjectsAgainstBucket(bucketObjects, bucket + Vector2i(x, y));
		}
	}	

	for (unsigned i = 0; i < bucketObjects.size(); ++i)
	{
		bucketObjects[i]->UpdateShape(*this);
	}
}

void World::UpdateTriangle(int id, const Triangle& triangle)
{
	_buffers[_writeBuffer]._triangles[id] = triangle;
}

void World::UpdateQuad(int id, const Quad& quad)
{
	_buffers[_writeBuffer]._quads[id] = quad;
}

const Quad* World::GetQuadDrawBuffer() const
{
	if (_buffers[_drawBuffer]._quads.empty())
		return NULL;

	return &(_buffers[_drawBuffer]._quads[0]);
}

const Triangle* World::GetTriangleDrawBuffer() const
{
	if (_buffers[_drawBuffer]._triangles.empty())
		return NULL;

	return &(_buffers[_drawBuffer]._triangles[0]);
}

int World::GetNumQuads() const
{
	return _buffers[_drawBuffer]._quads.size();
}

int World::GetNumTriangles() const
{
	return _buffers[_drawBuffer]._quads.size();
}

void World::Draw()
{
	SwapDrawState();

	_quadBuffer.SetShapes(GetQuadDrawBuffer(), GetNumQuads());
	_triangleBuffer.SetShapes(GetTriangleDrawBuffer(), GetNumTriangles());

	_shapeBatch.Draw();
}

void World::SwapDrawState()
{
	Threading::ScopedLock lock(_stateChangeMutex);

	if (_drawBuffer != _readBuffer)
	{
		_freeBuffer = _drawBuffer;
	}

	_drawBuffer = _readBuffer;
}

void World::SwapWriteState()
{
	Threading::ScopedLock lock(_stateChangeMutex);

	// The state we just wrote is now the readable state
	_readBuffer = _writeBuffer;

	// Write to the invalid state, which is either the one just wrote
	// or one that the renderer recently finished with
	_writeBuffer = _freeBuffer;

	// The previous readState will be invalid after the next step and  
	// will be written to at the next oppertunity
	_freeBuffer = _readBuffer;
}

void World::UpdateObject(int object, double delta)
{
	_objects[object]->Integrate(delta);
}

int World::GetNumObjects() const
{
	return _objects.size();
}

Vector2i World::GetBucketForPoint(const Vector2d& point) const
{
	 Vector2i bucket(Vector2d(GetNumBucketsWide(), GetNumBucketsTall()) * 
					 (point - _worldMin) / (_worldMax - _worldMin));

	 bucket.x( Util::Clamp(bucket.x(), 0, GetNumBucketsWide() - 1) );
	 bucket.y( Util::Clamp(bucket.y(), 0, GetNumBucketsTall() - 1) );

	 return bucket;
}

Vector2d World::GetBucketMin(int x, int y) const
{
	return Vector2d((double)x, (double)y) / Vector2d(GetNumBucketsWide(), GetNumBucketsTall())
				* (_worldMax - _worldMin) + _worldMin;
}

int World::GetBucketIndex(const Vector2i& bucket) const
{
	return bucket.x() + bucket.y() * GetNumBucketsWide();
}