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
		-0.5f, -0.5f,
		0.5f, -0.5f,
		0.5f, 0.5f,
		-0.5f, 0.5f,
	};

	const unsigned int quadIndices[] =
	{
		0, 1, 2,
		0, 2, 3,
	};

	_quadBuffer.Create(*renderer, quadVerts, sizeof(quadVerts));
	_quadIndices.Create(*renderer, quadIndices, sizeof(quadIndices));

	_vertexColorFrag.CreateFromFile(*renderer, "data/vertexColor.frag");

	_quadVertShader.CreateFromFile(*renderer, "data/quadBatch.vert");
	_quadShader.Create(*renderer, _quadVertShader, _vertexColorFrag);
	renderer->GetStandardUniforms(_quadShader, _quadUniforms);

	_triangleVertShader.CreateFromFile(*renderer, "data/triangleBatch.vert");
	_triangleGeomShader.CreateFromFile(*renderer, "data/triangleBatch.geom");
	_triangleShader.Create(*renderer, _triangleVertShader, _vertexColorFrag, _triangleGeomShader);
	renderer->GetStandardUniforms(_triangleShader, _triangleUniforms);

	_lineVertShader.CreateFromFile(*renderer, "data/lineBatch.vert");
	_lineGeomShader.CreateFromFile(*renderer, "data/lineBatch.geom");
	_lineShader.Create(*renderer, _lineVertShader, _vertexColorFrag, _lineGeomShader);
	renderer->GetStandardUniforms(_lineShader, _lineUniforms);
}

void ShapeBatch::Dispose()
{
	for (unsigned i = 0; i < _quadArrays.size(); ++i)
	{
		_quadArrays[i]->Dispose();
	}

	for (unsigned i = 0; i < _triangleArrays.size(); ++i)
	{
		_triangleArrays[i]->Dispose();
	}

	for (unsigned i = 0; i < _lineArrays.size(); ++i)
	{
		_lineArrays[i]->Dispose();
	}

	_quadShader.Dispose();
	_quadVertShader.Dispose();

	_quadBuffer.Dispose();
	_quadIndices.Dispose();

	_triangleShader.Dispose();
	_triangleVertShader.Dispose();
	_triangleGeomShader.Dispose();

	_lineShader.Dispose();
	_lineVertShader.Dispose();
	_lineGeomShader.Dispose();

	_vertexColorFrag.Dispose();
}

void ShapeBatch::Draw()
{
	_renderer->EnableCullFace(false);

	// Draw Quads
	_quadShader.Use();
	_renderer->UpdateStandardUniforms(_quadShader, _quadUniforms);

	for(unsigned int i = 0; i < _quadArrays.size(); i++)
	{
		DrawQuadArray(_quadArrays[i]);
	}

	// Draw Triangles
	_triangleShader.Use();
	_renderer->UpdateStandardUniforms(_triangleShader, _triangleUniforms);

	for (unsigned int i = 0; i < _triangleArrays.size(); ++i)
	{
		DrawTriangleArray(_triangleArrays[i]);
	}

	// Draw Lines
	_lineShader.Use();
	_renderer->UpdateStandardUniforms(_lineShader, _lineUniforms);

	for (unsigned i = 0; i < _lineArrays.size(); ++i)
	{
		DrawLineArray(_lineArrays[i]);
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

	_renderer->Draw(triangleArray->_bufferBinding, PT_POINTS, 0, triangleArray->GetCount());
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

void ShapeBatch::DrawLineArray(LineArray* lineArray)
{
	if (lineArray->GetCount() == 0)
		return;

	if (lineArray->_needsUpdate)
	{
		UpdateLineArrayBinding(lineArray);
		lineArray->_needsUpdate = false;
	}

	_renderer->Draw(lineArray->_bufferBinding, PT_POINTS, 0, lineArray->GetCount());
}

void ShapeBatch::UpdateLineArrayBinding(LineArray* lineArray)
{
	lineArray->_needsDisposing = true;

	const ArrayElement vertexLayout [] =
	{
		ArrayElement(lineArray->_instanceBuffer, "in_vert0", 2, AE_FLOAT, sizeof(Line), 0, 0),
		ArrayElement(lineArray->_instanceBuffer, "in_vert1", 2, AE_FLOAT, sizeof(Line), sizeof(float)*2, 0),
		ArrayElement(lineArray->_instanceBuffer, "in_color", 4, AE_UBYTE, sizeof(Line), sizeof(float)*4, 0), 
	};

	lineArray->_bufferBinding.Create(*_renderer, _lineShader, vertexLayout, 3); 
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

void ShapeBatch::AddArray(QuadArray* quadArray)
{
	AddShapeArray<QuadArray>(*_renderer, quadArray, _quadArrays);
}

void ShapeBatch::RemoveArray(QuadArray* quadArray)
{
	RemoveShapeArray<QuadArray>(quadArray, _quadArrays);
}

void ShapeBatch::AddArray(TriangleArray* triangleArray)
{
	AddShapeArray<TriangleArray>(*_renderer, triangleArray, _triangleArrays);
}

void ShapeBatch::RemoveArray(TriangleArray* triangleArray)
{
	RemoveShapeArray<TriangleArray>(triangleArray, _triangleArrays);
}

void ShapeBatch::AddArray(LineArray* lineArray)
{
	AddShapeArray<LineArray>(*_renderer, lineArray, _lineArrays);
}

void ShapeBatch::RemoveArray(LineArray* lineArray)
{
	RemoveShapeArray<LineArray>(lineArray, _lineArrays);
}