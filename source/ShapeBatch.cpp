// David Hart - 2012

#include "ShapeBatch.h"
#include "Util.h"
#include "Renderer.h"
#include <algorithm>

ShapeBatch::ShapeBatch() :
	_renderer(NULL)
{
}

void ShapeBatch::Create(const Renderer* renderer)
{
	_renderer = renderer;
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

	_quadBuffer.Create(*renderer, quadVerts, sizeof(quadVerts));
	_quadIndices.Create(*renderer, quadIndices, sizeof(quadIndices));

	_quadVertShader.CreateFromFile(*renderer, "data/quadBatch.vert");
	_quadFragShader.CreateFromFile(*renderer, "data/quadBatch.frag");
	_quadShader.Create(*renderer, _quadVertShader, _quadFragShader);

	renderer->GetStandardUniforms(_quadShader, _quadUniforms);

	_triangleVertShader.CreateFromFile(*renderer, "data/triangleBatch.vert");
	_triangleFragShader.CreateFromFile(*renderer, "data/triangleBatch.frag");
	_triangleGeomShader.CreateFromFile(*renderer, "data/triangleBatch.geom");
	_triangleShader.Create(*renderer, _triangleVertShader, _triangleFragShader, _triangleGeomShader);
	_triangleShader.Use();

	renderer->GetStandardUniforms(_triangleShader, _triangleUniforms);
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

void ShapeBatch::Draw()
{
	_renderer->EnableCullFace(false);

	_quadShader.Use();
	_renderer->UpdateStandardUniforms(_quadShader, _quadUniforms);

	for(unsigned int i = 0; i < _quadArrays.size(); i++)
	{
		DrawQuadArray(_quadArrays[i]);
	}

	_triangleShader.Use();
	_renderer->UpdateStandardUniforms(_triangleShader, _triangleUniforms);

	for (unsigned int i = 0; i < _triangleArrays.size(); ++i)
	{
		DrawTriangleArray(_triangleArrays[i]);
	}
}

void ShapeBatch::DrawQuadArray(QuadArray* quadArray)
{
	if (quadArray->GetCount() == 0)
		return;

	if (quadArray->_needsUpdate)
	{
		UpdateQuadArrayBinding(quadArray);
		quadArray->_needsUpdate = false;
	}

	_renderer->DrawInstances(quadArray->_bufferBinding, PT_TRIANGLES, 0, 6, quadArray->GetCount());
}

void ShapeBatch::UpdateQuadArrayBinding(QuadArray* quadArray)
{
	quadArray->_needsDisposing = true;

	const ArrayElement vertexLayout [] =
	{
		ArrayElement(_quadBuffer, "in_vertex", 2, AE_FLOAT, sizeof(float)*2, 0, 0),
		ArrayElement(quadArray->_instanceBuffer, "in_positionRotation", 3, AE_FLOAT, sizeof(Quad), 0, 1),
		ArrayElement(quadArray->_instanceBuffer, "in_color", 4, AE_UBYTE, sizeof(Quad), sizeof(float)*3, 1), 
	};

	quadArray->_bufferBinding.Create(*_renderer, _quadShader, vertexLayout, 3, _quadIndices, AE_UINT); 
}

void ShapeBatch::DrawTriangleArray(TriangleArray* triangleArray)
{
	if (triangleArray->GetCount() == 0)
		return;

	if (triangleArray->_needsUpdate)
	{
		UpdateTriangleArrayBinding(triangleArray);
		triangleArray->_needsUpdate = false;
	}

	_renderer->Draw(triangleArray->_bufferBinding, PT_POINTS, 0, 3 * triangleArray->GetCount());
}

void ShapeBatch::UpdateTriangleArrayBinding(TriangleArray* triangleArray)
{
	triangleArray->_needsDisposing = true;

	const ArrayElement vertexLayout [] =
	{
		ArrayElement(triangleArray->_instanceBuffer, "in_vert0", 2, AE_FLOAT, sizeof(Triangle), 0, 0),
		ArrayElement(triangleArray->_instanceBuffer, "in_vert1", 2, AE_FLOAT, sizeof(Triangle), sizeof(float)*2, 0),
		ArrayElement(triangleArray->_instanceBuffer, "in_vert2", 2, AE_FLOAT, sizeof(Triangle), sizeof(float)*4, 0),
		ArrayElement(triangleArray->_instanceBuffer, "in_color", 4, AE_UBYTE, sizeof(Triangle), sizeof(float)*6, 0), 
	};

	triangleArray->_bufferBinding.Create(*_renderer, _triangleShader, vertexLayout, 4); 
}

template <typename T> void ShapeBatch::AddShapeArray(const Renderer& renderer, T* shapeArray, std::vector<T*>& shapeArrays)
{
	assert(std::find(shapeArrays.begin(), shapeArrays.end(), shapeArray) == shapeArrays.end());
	shapeArray->_instanceBuffer.Create(renderer);

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
	AddShapeArray<QuadArray>(*_renderer, quadArray, _quadArrays);
}

void ShapeBatch::RemoveQuadArray(QuadArray* quadArray)
{
	RemoveShapeArray<QuadArray>(quadArray, _quadArrays);
}

void ShapeBatch::AddTriangleArray(TriangleArray* triangleArray)
{
	AddShapeArray<TriangleArray>(*_renderer, triangleArray, _triangleArrays);
}

void ShapeBatch::RemoveTriangleArray(TriangleArray* triangleArray)
{
	RemoveShapeArray<TriangleArray>(triangleArray, _triangleArrays);
}
