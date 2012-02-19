// David Hart - 2012

#pragma once

#include "ShaderProgram.h"
#include "VertexBinding.h"
#include "VertexBuffer.h"
#include "Shader.h"
#include "Vector2.h"
#include "Vector4.h"
#include <vector>
#include <cassert>

class Renderer;
class QuadArray;

struct Quad
{
	Vector2 _position;
	float _rotation;
	// TODO: Add Vector2 size?
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
	Uniform _quadBatchColor;

	std::vector<QuadArray*> _quadArrays;

};

class QuadArray
{

	friend class ShapeBatch;

public:
	
	QuadArray();
	void SetSize(unsigned int size);
	unsigned int GetSize();
	Quad& GetQuad(unsigned int index);
	void SetColor(const Vector4& color);
	Vector4 GetColor();

	void Dispose();

private:

	std::vector<Quad> _quads;
	VertexBuffer _quadInstanceBuffer;
	VertexBinding _bufferBinding;
	Vector4 _color;
	bool _needsDisposing;

};

inline void QuadArray::SetSize(unsigned int size)
{
	_quads.resize(size);
}

inline unsigned int QuadArray::GetSize()
{
	return _quads.size();
}

inline Quad& QuadArray::GetQuad(unsigned int index)
{
	assert(index < _quads.size());

	return _quads[index];
}

inline void QuadArray::SetColor(const Vector4& color)
{
	_color = color;
}

inline Vector4 QuadArray::GetColor()
{
	return _color;
}
