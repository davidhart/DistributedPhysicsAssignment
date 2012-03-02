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

struct Quad
{
	Vector2 _position;
	float _rotation;
	Color _color;
};

struct Triangle
{
	Vector2 _points[3];
	Color _color;
};

template<typename T> class ShapeArray
{
	friend class ShapeBatch;

public:
	
	ShapeArray() :
		_needsDisposing(false),
		_needsUpdate(false)
	{

	}

	~ShapeArray()
	{
		assert(!_needsDisposing);
	}

	inline void SetSize(unsigned int size)
	{
		_shapes.resize(size);
	}

	inline unsigned int GetSize()
	{
		return _shapes.size();
	}

	void SetShape(unsigned int index, const T& shape)
	{
		assert(index < _shapes.size());

		_shapes[index] = shape;
		_needsUpdate = true;
	}

	T GetShape(unsigned int index)
	{
		assert(index < _shapes.size());

		return _shapes[index];
	}

private:

	void Dispose()
	{
		if (_needsDisposing)
		{
			_bufferBinding.Dispose();
			_instanceBuffer.Dispose();
			_needsDisposing = false;
		}
	}

	std::vector<T> _shapes;
	VertexBuffer _instanceBuffer;
	VertexBinding _bufferBinding;
	bool _needsDisposing;
	bool _needsUpdate;

};

typedef ShapeArray<Quad> QuadArray;
typedef ShapeArray<Triangle> TriangleArray;

class ShapeBatch
{

public:

	void Create(const Renderer& renderer);
	void Dispose();
	void Draw(const Renderer& renderer);

	void AddQuadArray(QuadArray* quadArray);
	void RemoveQuadArray(QuadArray* quadArray);

	void AddTriangleArray(TriangleArray* triangleArray);
	void RemoveTriangleArray(TriangleArray* triangleArray);

private:

	template <typename T> static void AddShapeArray(T* shapeArray, std::vector<T*>& shapeArrays);
	template <typename T> static void RemoveShapeArray(T* shapeArray, std::vector<T*>& shapeArrays);

	void DrawQuadArray(const Renderer& renderer, QuadArray* quadArray);
	void UpdateQuadArray(const Renderer& renderer, QuadArray* quadArray);

	void DrawTriangleArray(const Renderer& renderer, TriangleArray* triangleArray);
	void UpdateTriangleArray(const Renderer& renderer, TriangleArray* triangleArray);

	VertexShader _quadVertShader;
	FragmentShader _quadFragShader;
	ShaderProgram _quadShader;
	VertexBuffer _quadBuffer;
	VertexBuffer _quadIndices;
	Renderer::StandardUniformBlock _quadUniforms;

	VertexShader _triangleVertShader;
	FragmentShader _triangleFragShader;
	GeometryShader _triangleGeomShader;
	ShaderProgram _triangleShader;
	Renderer::StandardUniformBlock _triangleUniforms;

	std::vector<QuadArray*> _quadArrays;
	std::vector<TriangleArray*> _triangleArrays;
};
