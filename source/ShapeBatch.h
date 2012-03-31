// David Hart - 2012
// TODO: Document

#pragma once

#include "ShaderProgram.h"
#include "VertexBinding.h"
#include "VertexBuffer.h"
#include "Renderer.h"
#include "Shader.h"
#include "Vector.h"
#include "Color.h"
#include <vector>
#include <cassert>

struct Quad
{
	Vector2f _position;
	float _rotation;
	Color _color;
};

struct Triangle
{
	Vector2f _points[3];
	Color _color;
};

struct Line
{
	Vector2f _points[2];
	Color _color;
};

template<typename T> class ShapeArray
{
	friend class ShapeBatch;

public:
	
	ShapeArray() :
		_count(0),
		_needsDisposing(false),
		_needsUpdate(false)
	{

	}

	~ShapeArray()
	{
		assert(!_needsDisposing);
	}

	inline void SetShapes(const T* shapes, unsigned int count)
	{
		int bufferSize = count * sizeof(T);

		// If we will resize the buffer then we will need to re-create the binding
		if (bufferSize != _instanceBuffer.Size())
			_needsUpdate = true;

		_instanceBuffer.SetData(shapes, bufferSize);
		_count = count;
	}

	inline unsigned int GetCount() const
	{
		return _count;
	}

private:

	void Dispose()
	{
		_instanceBuffer.Dispose();

		if (_needsDisposing)
		{
			_bufferBinding.Dispose();
			_needsDisposing = false;
		}
	}

	VertexBuffer _instanceBuffer;
	VertexBinding _bufferBinding;
	unsigned int _count;
	bool _needsDisposing;
	bool _needsUpdate;

};

typedef ShapeArray<Quad> QuadArray;
typedef ShapeArray<Triangle> TriangleArray;
typedef ShapeArray<Line> LineArray;

class ShapeBatch
{

public:

	ShapeBatch();

	void Create(const Renderer* renderer);
	void Dispose();
	void Draw();

	void AddArray(QuadArray* quadArray);
	void AddArray(TriangleArray* triangleArray);
	void AddArray(LineArray* lineArray);

	void RemoveArray(QuadArray* quadArray);
	void RemoveArray(TriangleArray* triangleArray);
	void RemoveArray(LineArray* lineArray);

private:

	template <typename T> static void AddShapeArray(const Renderer& renderer, T* shapeArray, std::vector<T*>& shapeArrays);
	template <typename T> static void RemoveShapeArray(T* shapeArray, std::vector<T*>& shapeArrays);

	void DrawQuadArray(QuadArray* quadArray);
	void UpdateQuadArrayBinding(QuadArray* quadArray);

	void DrawTriangleArray(TriangleArray* triangleArray);
	void UpdateTriangleArrayBinding(TriangleArray* triangleArray);

	void DrawLineArray(LineArray* lineArray);
	void UpdateLineArrayBinding(LineArray* lineArray);

	FragmentShader _vertexColorFrag;

	VertexShader _quadVertShader;
	ShaderProgram _quadShader;
	VertexBuffer _quadBuffer;
	VertexBuffer _quadIndices;
	Renderer::StandardUniformBlock _quadUniforms;

	VertexShader _triangleVertShader;
	GeometryShader _triangleGeomShader;
	ShaderProgram _triangleShader;
	Renderer::StandardUniformBlock _triangleUniforms;

	VertexShader _lineVertShader;
	GeometryShader _lineGeomShader;
	ShaderProgram _lineShader;
	Renderer::StandardUniformBlock _lineUniforms;

	std::vector<QuadArray*> _quadArrays;
	std::vector<TriangleArray*> _triangleArrays;
	std::vector<LineArray*> _lineArrays;

	const Renderer* _renderer;
};
