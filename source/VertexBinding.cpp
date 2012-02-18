// David Hart - 2011

#include "VertexBinding.h"
#include "VertexBuffer.h"
#include "ShaderProgram.h"
#include "Renderer.h"

#include <cassert>

ArrayElement::ArrayElement(const VertexBuffer& buffer, 
				const char* attribute, 
				unsigned int numComponents, 
				ElementType type,
				unsigned int stride, 
				unsigned int offset,
				unsigned int instanceStep) :
	buffer(buffer),
	attribute(attribute),
	numComponents(numComponents),
	type(type),
	stride(stride),
	offset(offset),
	instanceStep(instanceStep)
{
}

VertexBinding::VertexBinding() :
	_glex(NULL),
	_vaoHandle(0),
	_indicesType(AE_UBYTE),
	_hasIndices(false)
{
}

VertexBinding::~VertexBinding()
{
	assert(_vaoHandle == 0);
}

void VertexBinding::Create(const Renderer& renderer, const ShaderProgram& shaderProgram, const ArrayElement* elements, unsigned int numelements)
{
	CreateVAO(renderer);

	Bind();

	SetupAttribPointers(shaderProgram, elements, numelements);

	Unbind();
}

void VertexBinding::Create(const Renderer& renderer, const ShaderProgram& shaderProgram, const ArrayElement* elements, unsigned int numelements, const VertexBuffer& indices, ElementType indicesType)
{
	CreateVAO(renderer);

	Bind();

	SetupAttribPointers(shaderProgram, elements, numelements);
	SetupIndices(indices, indicesType);

	Unbind();
}

void VertexBinding::CreateVAO(const Renderer& renderer)
{
	if (_vaoHandle != 0)
		Dispose();

	_glex = renderer.GetEx();

	_glex->glGenVertexArrays(1, &_vaoHandle);
}

void VertexBinding::SetupAttribPointers(const ShaderProgram& shaderProram, const ArrayElement* elements, unsigned int numelements)
{
	for (unsigned int i = 0; i < numelements; ++i)
	{
		const ArrayElement& element = elements[i];

		// If the attrib location is less than zero GetAttribLocation must have returned -1
		// indicating the attribute doesn't exist so we ignore it
		int attribLocation = shaderProram.GetAttributeIndex(element.attribute);

		if (attribLocation < 0)
			continue;

		_glex->glBindBuffer(GL_ARRAY_BUFFER, element.buffer._vbHandle);

		GLenum type = GL_FLOAT;

		if (AE_FLOAT == element.type)
			type = GL_FLOAT;
		else if (AE_INT == element.type)
			type = GL_INT;

		_glex->glEnableVertexAttribArray(attribLocation);
		_glex->glVertexAttribPointer(attribLocation, element.numComponents, type, GL_FALSE, element.stride,
				(void*)element.offset);

		_glex->glVertexAttribDivisorARB(attribLocation, element.instanceStep);
	}
}

void VertexBinding::SetupIndices(const VertexBuffer& indices, ElementType indicesType)
{
	_hasIndices = true;
	_indicesType = indicesType;

	assert(indicesType == AE_UBYTE || indicesType == AE_UINT || indicesType == AE_USHORT);

	_glex->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices._vbHandle);
}

void VertexBinding::Dispose()
{
	assert(_vaoHandle != 0);

	_glex->glDeleteVertexArrays(1, &_vaoHandle);
	_vaoHandle = 0;
	_glex = NULL;
	_hasIndices = false;
}

void VertexBinding::Bind()
{
	assert(_vaoHandle != 0);

	_glex->glBindVertexArray(_vaoHandle);
}

void VertexBinding::Unbind()
{
	_glex->glBindVertexArray(0);
}