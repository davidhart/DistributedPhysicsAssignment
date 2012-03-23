// David Hart - 2011
//
// class VertexBuffer
//   VertexBuffer represents a wrapper around OpenGL VBOs. VertexBuffers may also
//   contain index data for use as an index buffer with the VertexBinding class.
// 

#pragma once

#include "Uncopyable.h"
#include "glex.h"

class Renderer;

class VertexBuffer : public Uncopyable
{
	friend class VertexBinding;

public:

	explicit VertexBuffer();
	~VertexBuffer();

	// Create an empty buffer
	void Create(const Renderer& renderer);

	// Create and initialise
	void Create(const Renderer& renderer, const void* data, unsigned int size);

	// Set contents and resize
	void SetData(const void* data, unsigned int size);

	// Update range of data
	void UpdateRegion(unsigned int offset, const void* newdata, unsigned int size);
	void Dispose();

	inline unsigned int Size() const { return _vbSize; }

private:

	glex* _glex;
	unsigned int _vbSize;
	GLuint _vbHandle;
};