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
}

void ShapeBatch::Dispose()
{
	for (unsigned int i = 0; i < _quadArrays.size(); ++i)
	{
		_quadArrays[i]->Dispose();
	}

	_quadShader.Dispose();
	_quadVertShader.Dispose();
	_quadFragShader.Dispose();

	_quadBuffer.Dispose();
	_quadIndices.Dispose();
}

void ShapeBatch::Draw(const Renderer& renderer)
{
	if (_quadArrays.size() == 0)
		return;

	_quadShader.Use();

	renderer.EnableCullFace(false);

	for(unsigned int i = 0; i < _quadArrays.size(); i++)
	{
		DrawQuadArray(renderer, _quadArrays[i]);
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

	renderer.UpdateStandardUniforms(_quadShader, _quadUniforms);

	renderer.DrawInstances(quadArray->_bufferBinding, PT_TRIANGLES, 0, 6, quadArray->GetSize());
}

void ShapeBatch::UpdateQuadArray(const Renderer& renderer, QuadArray* quadArray)
{
	unsigned int arraySizeInBytes = quadArray->GetSize() * sizeof(Quad);

	if (arraySizeInBytes != quadArray->_quadInstanceBuffer.Size())
	{
		quadArray->_needsDisposing = true;

		quadArray->_quadInstanceBuffer.Create(renderer, &(quadArray->_quads[0]), arraySizeInBytes);

		const ArrayElement vertexLayout [] =
		{
			ArrayElement(_quadBuffer, "in_vertex", 2, AE_FLOAT, sizeof(float)*2, 0, 0),
			ArrayElement(quadArray->_quadInstanceBuffer, "in_positionRotation", 3, AE_FLOAT, sizeof(Quad), 0, 1),
			ArrayElement(quadArray->_quadInstanceBuffer, "in_color", 4, AE_UBYTE, sizeof(Quad), sizeof(float)*3, 1), 
		};

		quadArray->_bufferBinding.Create(renderer, _quadShader, vertexLayout, 3, _quadIndices, AE_UINT); 
	}
	else
	{
		quadArray->_quadInstanceBuffer.UpdateRegion(0, &(quadArray->_quads[0]), arraySizeInBytes);
	}
}

void ShapeBatch::AddQuadArray(QuadArray* quadArray)
{
	assert(std::find(_quadArrays.begin(), _quadArrays.end(), quadArray) == _quadArrays.end());

	_quadArrays.push_back(quadArray);
}

void ShapeBatch::RemoveQuadArray(QuadArray* quadArray)
{
	std::vector<QuadArray*>::iterator it = std::find(_quadArrays.begin(), _quadArrays.end(), quadArray);

	if (it == _quadArrays.end())
	{
		assert(false);
		return;
	}

	(*it)->Dispose();
	_quadArrays.erase(it);
}

QuadArray::QuadArray() :
	_needsDisposing(false),
	_needsUpdate(false)
{

}

QuadArray::~QuadArray()
{
	assert(!_needsDisposing);
}

void QuadArray::Dispose()
{
	if (_needsDisposing)
	{
		_bufferBinding.Dispose();
		_quadInstanceBuffer.Dispose();
		_needsDisposing = false;
	}
}
