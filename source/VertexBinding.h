// David Hart - 2011
//
// class VertexBinding
//   VertexBinding represents a wrapper around the OpenGL 3.0 Vertex
//   Array Object which allows efficient re-binding of Vertex Buffers.
//

#pragma once

#include "Uncopyable.h"
#include "glex.h"

class ShaderProgram;
class VertexBuffer;
class Renderer;

enum ElementType
{
	AE_FLOAT = GL_FLOAT,
	AE_INT = GL_INT,

	AE_UBYTE = GL_UNSIGNED_BYTE,
	AE_UINT = GL_UNSIGNED_INT,
	AE_USHORT = GL_UNSIGNED_SHORT,
};

struct ArrayElement
{
	const VertexBuffer& buffer;
	const char* attribute;
	unsigned int numComponents;
	ElementType type;
	unsigned int stride;
	unsigned int offset;
	unsigned int instanceStep;

	ArrayElement(const VertexBuffer& buffer, 
				const char* attribute, 
				unsigned int numComponents, 
				ElementType type,
				unsigned int stride, 
				unsigned int offset,
				unsigned int instanceStep = 0);
};

class VertexBinding : public Uncopyable
{
	friend class Renderer;

public:

	explicit VertexBinding();
	~VertexBinding();
	void Create(const Renderer& renderer, const ShaderProgram& program, const ArrayElement* elements, unsigned int numelements);
	void Create(const Renderer& renderer, const ShaderProgram& program, const ArrayElement* elements, unsigned int numelements, const VertexBuffer& indices, ElementType indicesType);
	void Dispose();

	void Bind();
	void Unbind();

private:

	void CreateVAO(const Renderer& renderer);
	void SetupAttribPointers(const ShaderProgram& shaderProgram, const ArrayElement* elements, unsigned int numelements);
	void SetupElementPointers(const VertexBuffer& buffer);
	void SetupIndices(const VertexBuffer& indices, ElementType indicesType);

	glex* _glex;
	GLuint _vaoHandle;
	ElementType _indicesType;
	bool _hasIndices;

};