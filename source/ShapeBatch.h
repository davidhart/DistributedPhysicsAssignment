// David Hart - 2012

#pragma once

#include "ShaderProgram.h"
#include "VertexBinding.h"
#include "VertexBuffer.h"
#include "Renderer.h"
#include "Shader.h"
#include "Vector2.h"
#include "Vector4.h"
#include "Color.h"
#include <vector>
#include <cassert>

class QuadArray;

struct Quad
{
	Vector2 _position;
	float _rotation;
	Color _color;
};

class ShapeBatch
{

public:

	void Create(const Renderer& renderer);
	void Dispose();
	void Draw(const Renderer& renderer);

	void AddQuadArray(QuadArray* quadArray);
	void RemoveQuadArray(QuadArray* quadArray);

private:

	void DrawQuadArray(const Renderer& renderer, QuadArray* quadArray);
	void UpdateQuadArray(const Renderer& renderer, QuadArray* quadArray);

	VertexShader _quadVertShader;
	FragmentShader _quadFragShader;
	ShaderProgram _quadShader;
	VertexBuffer _quadBuffer;
	VertexBuffer _quadIndices;
	Renderer::StandardUniformBlock _quadUniforms;

	std::vector<QuadArray*> _quadArrays;

};

class QuadArray
{

	friend class ShapeBatch;

public:
	
	QuadArray();
	~QuadArray();
	void SetSize(unsigned int size);
	unsigned int GetSize();
	void SetQuad(unsigned int index, const Quad& quad);
	Quad GetQuad(unsigned int index);

private:

	void Dispose();

	std::vector<Quad> _quads;
	VertexBuffer _quadInstanceBuffer;
	VertexBinding _bufferBinding;
	bool _needsDisposing;
	bool _needsUpdate;

};

inline void QuadArray::SetSize(unsigned int size)
{
	_quads.resize(size);
}

inline unsigned int QuadArray::GetSize()
{
	return _quads.size();
}

inline void QuadArray::SetQuad(unsigned int index, const Quad& quad)
{
	assert(index < _quads.size());

	_quads[index] = quad;
	_needsUpdate = true;
}

inline Quad QuadArray::GetQuad(unsigned int index)
{
	assert(index < _quads.size());

	return _quads[index];
}