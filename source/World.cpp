#include "World.h"

World::World() :
	_drawBuffer(0),
	_readBuffer(0),
	_writeBuffer(1),
	_freeBuffer(2)
{
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

	_worldBoundaryLines.resize(4);
	
	Line l;
	l._color = Color(1.0f, 0.0f, 0.0f, 1.0f);
	l._points[0] = Vector2f(-20, 20);
	l._points[1] = Vector2f(-20, 0);

	_worldBoundaryLines[0] = l;

	l._points[0].x(20);
	l._points[1].x(20);
	_worldBoundaryLines[1] = l;

	l._points[0] = Vector2f(-20, 20);
	l._points[1] = Vector2f(20, 20);
	_worldBoundaryLines[2] = l;

	l._points[0].y(0);
	l._points[1].y(0);
	_worldBoundaryLines[3] = l;

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
	_objects[object]->UpdateShape(*this);
}

int World::GetNumObjects() const
{
	return _objects.size();
}
