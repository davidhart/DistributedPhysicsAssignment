#include "WorldState.h"

WorldState::WorldState() :
	_drawBuffer(0),
	_readBuffer(0),
	_writeBuffer(1),
	_freeBuffer(2)
{
}

WorldState::~WorldState()
{
	for (unsigned i = 0; i < _objects.size(); ++i)
	{
		delete _objects[i];
	}
}

Physics::TriangleObject* WorldState::AddTriangle()
{
	for (int i = 0; i < NUM_STATE_BUFFERS; i++)
	{
		_buffers[i]._triangles.push_back(Triangle());
	}

	Physics::TriangleObject* triangle = new Physics::TriangleObject(_buffers[0]._triangles.size() - 1);

	_objects.push_back(triangle);

	return triangle;
}

Physics::BoxObject* WorldState::AddBox()
{
	for (int i = 0; i < NUM_STATE_BUFFERS; i++)
	{
		_buffers[i]._quads.push_back(Quad());
	}

	Physics::BoxObject* box = new Physics::BoxObject(_buffers[0]._quads.size() - 1);

	_objects.push_back(box);

	return box;
}

void WorldState::UpdateTriangle(int id, const Triangle& triangle)
{
	_buffers[_writeBuffer]._triangles[id] = triangle;
}

void WorldState::UpdateQuad(int id, const Quad& quad)
{
	_buffers[_writeBuffer]._quads[id] = quad;
}

const Quad* WorldState::GetQuadDrawBuffer() const
{
	if (_buffers[_drawBuffer]._quads.empty())
		return NULL;

	return &(_buffers[_drawBuffer]._quads[0]);
}

const Triangle* WorldState::GetTriangleDrawBuffer() const
{
	if (_buffers[_drawBuffer]._triangles.empty())
		return NULL;

	return &(_buffers[_drawBuffer]._triangles[0]);
}

int WorldState::GetNumQuads() const
{
	return _buffers[_drawBuffer]._quads.size();
}

int WorldState::GetNumTriangles() const
{
	return _buffers[_drawBuffer]._quads.size();
}

void WorldState::SwapDrawState()
{
	_stateChangeMutex.Enter();

	if (_drawBuffer != _readBuffer)
	{
		_freeBuffer = _drawBuffer;
	}

	_drawBuffer = _readBuffer;

	_stateChangeMutex.Exit();
}

void WorldState::SwapWriteState()
{
	_stateChangeMutex.Enter();

	// The state we just wrote is now the readable state
	_readBuffer = _writeBuffer;

	// Write to the invalid state, which is either the one just wrote
	// or one that the renderer recently finished with
	_writeBuffer = _freeBuffer;

	// The previous readState will be invalid after the next step and  
	// will be written to at the next oppertunity
	_freeBuffer = _readBuffer;

	_stateChangeMutex.Exit();
}

void WorldState::UpdateObject(int object, double delta)
{
	_objects[object]->Integrate(delta);
	_objects[object]->UpdateShape(*this);
}

int WorldState::GetNumObjects() const
{
	return _objects.size();
}
