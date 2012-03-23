// David Hart - 2011

#include "VertexBuffer.h"
#include "Renderer.h"

#include <cassert>
#include <exception>

const GLenum BUFFER_USAGE = GL_STREAM_DRAW;

VertexBuffer::VertexBuffer() :
	_glex(NULL),
	_vbSize(0),
	_vbHandle(0)
{
}

VertexBuffer::~VertexBuffer()
{
	assert(_vbHandle == 0);
}

void VertexBuffer::Create(const Renderer& renderer)
{
	if (_vbHandle != 0)
		Dispose();

	assert(_vbHandle == 0);

	_glex = renderer.GetEx();

	_glex->glGenBuffers(1, &_vbHandle);
	_vbSize = 0;

	assert(_vbHandle != 0);
}

void VertexBuffer::Create(const Renderer& renderer, const void* data, unsigned int size)
{
	if (_vbHandle != 0)
		Dispose();

	assert(_vbHandle == 0);
	assert(data != NULL);

	_glex = renderer.GetEx();

	_glex->glGenBuffers(1, &_vbHandle);
	_vbSize = size;

	assert(_vbHandle != 0);

	_glex->glBindBuffer(GL_ARRAY_BUFFER, _vbHandle);

	_glex->glBufferData(GL_ARRAY_BUFFER, size, (void*)data, BUFFER_USAGE);

	_glex->glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::SetData(const void* data, unsigned int size)
{
	// No need to resize buffer if the size matches
	if (size == _vbSize)
	{
		UpdateRegion(0, data, size);
		return;
	}

	assert(_vbHandle != 0);

	_glex->glBindBuffer(GL_ARRAY_BUFFER, _vbHandle);
	_glex->glBufferData(GL_ARRAY_BUFFER, size, data, BUFFER_USAGE);

	_vbSize = size;
}

void VertexBuffer::UpdateRegion(unsigned int offset, const void* newdata, unsigned int size)
{
	assert(_vbHandle != 0);
	assert(offset + size <= _vbSize);

	_glex->glBindBuffer(GL_ARRAY_BUFFER, _vbHandle);

	_glex->glBufferSubData(GL_ARRAY_BUFFER, offset, size, (void*)newdata);
}

void VertexBuffer::Dispose()
{
	assert(_vbHandle != 0);

	_glex->glDeleteBuffers(1, &_vbHandle);

	_vbHandle = 0;
	_vbSize = 0;
	_glex = NULL;
}