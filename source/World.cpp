#include "World.h"

World::World() :
	_drawBuffer(0),
	_readBuffer(0),
	_writeBuffer(1),
	_freeBuffer(2),
	_worldMin(-20, 0),
	_worldMax(20, 20),
	_objectTiedToCursor(NULL)
{
	_bucketSize = (_worldMax - _worldMin) / Vector2d(GetNumBucketsWide(), GetNumBucketsTall());
	_objectBuckets.resize(GetNumBucketsTall()*GetNumBucketsWide());


	_cursorSpring.SetSpringConstant(1000);
	_cursorSpring.SetDampingConstant(100);
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
	l._points[0].y((float)_worldMin.y());
	l._points[1].y((float)_worldMax.y());

	for (int i = 1; i < GetNumBucketsWide(); ++i)
	{
		l._points[0].x((float)GetBucketMin(i, 0).x());
		l._points[1].x(l._points[0].x());
		_worldBoundaryLines[4 + i - 1] = l;
	}

	l._points[0].x((float)_worldMin.x());
	l._points[1].x((float)_worldMax.x());

	for (int i = 1; i < GetNumBucketsTall(); ++i)
	{
		l._points[0].y((float)GetBucketMin(0, i).y());
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
	_buffers[_writeBuffer]._triangles.push_back(Triangle());

	Physics::TriangleObject* triangle = new Physics::TriangleObject(_buffers[_writeBuffer]._triangles.size() - 1);

	_objects.push_back(triangle);

	return triangle;
}

Physics::BoxObject* World::AddBox()
{
	_buffers[_writeBuffer]._quads.push_back(Quad());

	Physics::BoxObject* box = new Physics::BoxObject(_buffers[_writeBuffer]._quads.size() - 1);

	_objects.push_back(box);

	return box;
}

void World::ClearObjects()
{

	for (unsigned int i = 0; i < _objects.size(); ++i)
	{
		delete _objects[i];
	}

	_objects.clear();

	_buffers[_writeBuffer]._quads.clear();
	_buffers[_writeBuffer]._triangles.clear();
}

Physics::PhysicsObject* World::GetObject(int id)
{
	return _objects[id];
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

	bool xMin = bucketXMin == 0;
	bool xMax = bucketXMax == GetNumBucketsWide() - 1;

	for (unsigned i = 0; i < _objects.size(); ++i)
	{
		Vector2d position = _objects[i]->GetPosition();

		// If the position is somewhere within the band this thread is concerned with
		if (   (position.x() >= bucketXMinWorldSpace || xMin)
			&& (position.x() <  bucketXMaxWorldSpace || xMax) )
		{
			Vector2i bucket = GetBucketForPoint(position);

			assert(bucket.x() >= bucketXMin);
			assert(bucket.x() <= bucketXMax);

			_objectBuckets[GetBucketIndex(bucket)].push_back(i);
		}
	}
}

void World::SolveCollisions(int bucketXMin, int bucketXMax)
{
	Vector2i bucket;
	for (bucket.x(bucketXMin); bucket.x() <= bucketXMax; bucket.x(bucket.x() + 1))
	{
		for (bucket.y(0); bucket.y() < GetNumBucketsTall(); bucket.y(bucket.y() + 1))
		{
			SolveCollisionsInBucket(bucket);
		}
	}
}

void World::DetectCollisions(int bucketXMin, int bucketXMax)
{
	Vector2i bucket;
	for (bucket.x(bucketXMin); bucket.x() <= bucketXMax; bucket.x(bucket.x() + 1))
	{
		for (bucket.y(0); bucket.y() < GetNumBucketsTall(); bucket.y(bucket.y() + 1))
		{
			DetectCollisionsInBucket(bucket);
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

	Physics::Contact contact;

	for (unsigned i = 0; i < objects.size(); ++i)
	{
		for (unsigned j = 0; j < testBucket.size(); ++j)
		{
			Physics::PhysicsObject* object = _objects[objects[i]];

			if (object->TestCollision(*_objects[testBucket[j]], contact))
			{
				object->AddContact(contact);
			}
		}
	}
}

void World::DetectCollisionsInBucket(const Vector2i& bucket)
{
	Bucket& bucketObjects = _objectBuckets[GetBucketIndex(bucket)];
	
	Physics::Contact contact;

	for (unsigned i = 0; i < bucketObjects.size(); ++i)
	{
		for (unsigned j = i + 1; j < bucketObjects.size(); ++j)
		{
			Physics::PhysicsObject* objectA = _objects[bucketObjects[i]];
			Physics::PhysicsObject* objectB = _objects[bucketObjects[j]];

			if (objectA->TestCollision(*objectB, contact))
			{
				objectA->AddContact(contact);

				contact.Reverse();
				objectB->AddContact(contact);
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
}

void World::SolveCollisionsInBucket(const Vector2i& bucket)
{
	Bucket& bucketObjects = _objectBuckets[GetBucketIndex(bucket)];

	for (unsigned i = 0; i < bucketObjects.size(); ++i)
	{
		Physics::PhysicsObject* object = _objects[bucketObjects[i]];
		object->SolveContacts();
		object->UpdateShape(*this);
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

	_quadBuffer.SetShapes(GetQuadDrawBuffer(), _buffers[_drawBuffer]._quads.size());
	_triangleBuffer.SetShapes(GetTriangleDrawBuffer(), _buffers[_drawBuffer]._triangles.size());

	_shapeBatch.Draw();
}

void World::SwapDrawState()
{
	Threading::ScopedLock lock(_stateChangeMutex);

	if (_drawBuffer != _readBuffer)
	{
		// If the buffers were resized while we were drawing update the old draw buffer to the new size
		if (_buffers[_drawBuffer]._quads.size() != _buffers[_writeBuffer]._quads.size())
		{
			_buffers[_drawBuffer]._quads.resize(_buffers[_writeBuffer]._quads.size());
		}

		if (_buffers[_drawBuffer]._triangles.size() != _buffers[_writeBuffer]._triangles.size())
		{
			_buffers[_drawBuffer]._triangles.resize(_buffers[_writeBuffer]._triangles.size());
		}

		_freeBuffer = _drawBuffer;
	}

	_drawBuffer = _readBuffer;
}

void World::SwapWriteState()
{
	Threading::ScopedLock lock(_stateChangeMutex);

	// If the buffers were resized while we were drawing update the old freeBuffer buffer to the new size
	if (_buffers[_freeBuffer]._quads.size() != _buffers[_writeBuffer]._quads.size())
	{
		_buffers[_freeBuffer]._quads.resize(_buffers[_writeBuffer]._quads.size());
	}

	if (_buffers[_freeBuffer]._triangles.size() != _buffers[_writeBuffer]._triangles.size())
	{
		_buffers[_freeBuffer]._triangles.resize(_buffers[_writeBuffer]._triangles.size());
	}

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

	Vector2d p(Util::Clamp(point.x(), _worldMin.x(), _worldMax.x()-Util::EPSILON),
		Util::Clamp(point.y(), _worldMin.y(), _worldMax.y()-Util::EPSILON));

	 Vector2i bucket(Vector2d(GetNumBucketsWide(), GetNumBucketsTall()) * 
					 (p - _worldMin) / (_worldMax - _worldMin));

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

void World::HandleUserInteraction()
{
	Threading::ScopedLock lock(_userInteractionMutex);

	_cursorSpring.SetFixedEndpoint(_cursor);

	if (_leftButton && _objectTiedToCursor == NULL)
	{
		Physics::PhysicsObject* object = FindObjectAtPoint(_cursor);
		if (object != NULL)
		{
			// TODO: Worldspace -> object space function
			_cursorSpring.SetObjectSpaceAttachmentPoint(_cursor - object->GetPosition());
			object->SetColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
			object->AddConstraint(&_cursorSpring);
			_objectTiedToCursor = object;
		}
		/*
		else
		{
			Physics::PhysicsObject* object = AddBox();
			object->SetPosition(_cursor);
		}
		*/
	}

	if (!_leftButton && _objectTiedToCursor != NULL)
	{
		_objectTiedToCursor->RemoveConstraint(&_cursorSpring);
		_objectTiedToCursor = NULL;
		std::cout << " ====== " << std::endl;
	}
}

void World::UpdateMouseInput(const Vector2d& cursorPosition, bool leftButton, bool rightButton)
{
	Threading::ScopedLock lock(_userInteractionMutex);

	_cursor = cursorPosition;
	_leftButton = leftButton;
	_rightButton = rightButton;
}

Physics::PhysicsObject* World::FindObjectAtPoint(const Vector2d& point)
{
	Vector2i bucketCoord = GetBucketForPoint(point);
	Physics::PhysicsObject* object = NULL;

	for (int x = Util::Max(bucketCoord.x() - 1, 0); x < bucketCoord.x() + 2 && x < GetNumBucketsWide(); x++)
	{
		for (int y = Util::Max(bucketCoord.y() - 1, 0); y < bucketCoord.y() + 2 && y < GetNumBucketsTall(); y++)
		{
			object = FindObjectAtPointInBucket(point, Vector2i(x, y));

			if (object != NULL) 
				return object;
		}
	}

	return NULL;
}

Physics::PhysicsObject* World::FindObjectAtPointInBucket(const Vector2d& point, const Vector2i& bucketCoord)
{
	Bucket& objects = _objectBuckets[GetBucketIndex(bucketCoord)];

	for (unsigned i = 0; i < objects.size(); ++i)
	{
		// TODO: intersection test method in physics object
		Physics::PhysicsObject* object = _objects[objects[i]];

		Vector2d position = object->GetPosition();

		if (point.x() < position.x() + 0.5 &&
			point.x() > position.x() - 0.5 &&
			point.y() < position.y() + 0.5 &&
			point.y() > position.y() - 0.5)
		{
			return object;
		}
	}

	return NULL;
}

const std::vector<unsigned>& World::GetObjectsInBucket(int x, int y)
{
	return _objectBuckets[ GetBucketIndex( Vector2i(x, y) ) ];
}