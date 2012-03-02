// David Hart 2012

#include "ShapeBatch.h"
#include "Util.h"
#include "Renderer.h"
#include <algorithm>

void ShapeBatch::Create(const Renderer& renderer)
{
	const float quadVerts [] =
	{
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
	};

	const unsigned int quadIndices[] =
	{
		0, 1, 2,
		0, 2, 3,
	};

	_quadBuffer.Create(renderer, quadVerts, sizeof(quadVerts));
	_quadIndices.Create(renderer, quadIndices, sizeof(quadIndices));

	_quadVertShader.CreateFromFile(renderer, "data/quadBatch.vert");
	_quadFragShader.CreateFromFile(renderer, "data/quadBatch.frag");
	_quadShader.Create(renderer, _quadVertShader, _quadFragShader);

	renderer.GetStandardUniforms(_quadShader, _quadUniforms);

	_triangleVertShader.CreateFromFile(renderer, "data/triangleBatch.vert");
	_triangleFragShader.CreateFromFile(renderer, "data/triangleBatch.frag");
	_triangleGeomShader.CreateFromFile(renderer, "data/triangleBatch.geom");
	_triangleShader.Create(renderer, _triangleVertShader, _triangleFragShader, _triangleGeomShader);
	_triangleShader.Use();

	renderer.GetStandardUniforms(_triangleShader, _triangleUniforms);
}

void ShapeBatch::Dispose()
{
	for (unsigned int i = 0; i < _quadArrays.size(); ++i)
	{
		_quadArrays[i]->Dispose();
	}

	for (unsigned int i = 0; i < _triangleArrays.size(); ++i)
	{
		_triangleArrays[i]->Dispose();
	}

	_quadShader.Dispose();
	_quadVertShader.Dispose();
	_quadFragShader.Dispose();

	_quadBuffer.Dispose();
	_quadIndices.Dispose();

	_triangleShader.Dispose();
	_triangleVertShader.Dispose();
	_triangleFragShader.Dispose();
	_triangleGeomShader.Dispose();
}

void ShapeBatch::Draw(const Renderer& renderer)
{
	renderer.EnableCullFace(false);

	_quadShader.Use();
	renderer.UpdateStandardUniforms(_quadShader, _quadUniforms);

	for(unsigned int i = 0; i < _quadArrays.size(); i++)
	{
		DrawQuadArray(renderer, _quadArrays[i]);
	}

	_triangleShader.Use();
	renderer.UpdateStandardUniforms(_triangleShader, _triangleUniforms);

	for (unsigned int i = 0; i < _triangleArrays.size(); ++i)
	{
		DrawTriangleArray(renderer, _triangleArrays[i]);
	}
}

void ShapeBatch::DrawQuadArray(const Renderer& renderer, QuadArray* quadArray)
{
	if (quadArray->GetSize() == 0)
		return;

	if (quadArray->_needsUpdate)
	{
		UpdateQuadArray(renderer, quadArray);
		quadArray->_needsUpdate = false;
	}

	renderer.DrawInstances(quadArray->_bufferBinding, PT_TRIANGLES, 0, 6, quadArray->GetSize());
}

void ShapeBatch::UpdateQuadArray(const Renderer& renderer, QuadArray* quadArray)
{
	unsigned int arraySizeInBytes = quadArray->GetSize() * sizeof(Quad);

	if (arraySizeInBytes != quadArray->_instanceBuffer.Size())
	{
		quadArray->_needsDisposing = true;

		quadArray->_instanceBuffer.Create(renderer, &(quadArray->_shapes[0]), arraySizeInBytes);

		const ArrayElement vertexLayout [] =
		{
			ArrayElement(_quadBuffer, "in_vertex", 2, AE_FLOAT, sizeof(float)*2, 0, 0),
			ArrayElement(quadArray->_instanceBuffer, "in_positionRotation", 3, AE_FLOAT, sizeof(Quad), 0, 1),
			ArrayElement(quadArray->_instanceBuffer, "in_color", 4, AE_UBYTE, sizeof(Quad), sizeof(float)*3, 1), 
		};

		quadArray->_bufferBinding.Create(renderer, _quadShader, vertexLayout, 3, _quadIndices, AE_UINT); 
	}
	else
	{
		quadArray->_instanceBuffer.UpdateRegion(0, &(quadArray->_shapes[0]), arraySizeInBytes);
	}
}

void ShapeBatch::DrawTriangleArray(const Renderer& renderer, TriangleArray* triangleArray)
{
	if (triangleArray->GetSize() == 0)
		return;

	if (triangleArray->_needsUpdate)
	{
		UpdateTriangleArray(renderer, triangleArray);
		triangleArray->_needsUpdate = false;
	}

	renderer.Draw(triangleArray->_bufferBinding, PT_POINTS, 0, 3 * triangleArray->GetSize());
}

void ShapeBatch::UpdateTriangleArray(const Renderer& renderer, TriangleArray* triangleArray)
{
	unsigned int arraySizeInBytes = triangleArray->GetSize() * sizeof(Triangle);

	if (arraySizeInBytes != triangleArray->_instanceBuffer.Size())
	{
		triangleArray->_needsDisposing = true;

		triangleArray->_instanceBuffer.Create(renderer, &(triangleArray->_shapes[0]), arraySizeInBytes);

		const ArrayElement vertexLayout [] =
		{
			ArrayElement(triangleArray->_instanceBuffer, "in_vert0", 2, AE_FLOAT, sizeof(Triangle), 0, 0),
			ArrayElement(triangleArray->_instanceBuffer, "in_vert1", 2, AE_FLOAT, sizeof(Triangle), sizeof(float)*2, 0),
			ArrayElement(triangleArray->_instanceBuffer, "in_vert2", 2, AE_FLOAT, sizeof(Triangle), sizeof(float)*4, 0),
			ArrayElement(triangleArray->_instanceBuffer, "in_color", 4, AE_UBYTE, sizeof(Triangle), sizeof(float)*6, 0), 
		};

		triangleArray->_bufferBinding.Create(renderer, _triangleShader, vertexLayout, 4); 
	}
	else
	{
		triangleArray->_instanceBuffer.UpdateRegion(0, &(triangleArray->_shapes[0]), arraySizeInBytes);
	}
}

template <typename T> void ShapeBatch::AddShapeArray(T* shapeArray, std::vector<T*>& shapeArrays)
{
	assert(std::find(shapeArrays.begin(), shapeArrays.end(), shapeArray) == shapeArrays.end());

	shapeArrays.push_back(shapeArray);
}

template <typename T> void ShapeBatch::RemoveShapeArray(T* shapeArray, std::vector<T*>& shapeArrays)
{
	std::vector<T*>::iterator it = std::find(shapeArrays.begin(), shapeArrays.end(), shapeArray);

	if (it == shapeArrays.end())
	{
		assert(false);
		return;
	}

	(*it)->Dispose();
	shapeArrays.erase(it);
}

void ShapeBatch::AddQuadArray(QuadArray* quadArray)
{
	AddShapeArray<QuadArray>(quadArray, _quadArrays);
}

void ShapeBatch::RemoveQuadArray(QuadArray* quadArray)
{
	RemoveShapeArray<QuadArray>(quadArray, _quadArrays);
}

void ShapeBatch::AddTriangleArray(TriangleArray* triangleArray)
{
	AddShapeArray<TriangleArray>(triangleArray, _triangleArrays);
}

void ShapeBatch::RemoveTriangleArray(TriangleArray* triangleArray)
{
	RemoveShapeArray<TriangleArray>(triangleArray, _triangleArrays);
}
