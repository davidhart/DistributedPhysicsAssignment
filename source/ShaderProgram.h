// David Hart - 2011
// 
// class ShaderProgram
//   ShaderProgram is a wrapper around the OpenGL ShaderProgram object.
//   ShaderPrograms are linked on creation and thus do not support specification
//   of attribute locations. Instead the application should query the shader using
//   GetAttributeIndex.
// 

#pragma once

#include "Uncopyable.h"
#include "GXBase.h"
#include "Shader.h"

class Matrix4;

#include "Vector.h"

class ShaderProgram;

class Uniform
{

	friend class ShaderProgram;

public:

	Uniform();	

private:

	explicit Uniform(GLint uniformLocation);

	GLint _location;

};

class ShaderProgram : public Uncopyable
{

public:

	ShaderProgram();
	~ShaderProgram();

	void Create(const Renderer& renderer, const VertexShader& vertexShader, const FragmentShader& fragmentShader);
	void Create(const Renderer& renderer, const VertexShader& vertexShader, const FragmentShader& fragmentShader, const GeometryShader& geometryShader);
	void Dispose();

	int GetAttributeIndex(const char* attribute) const;
	Uniform GetUniform(const char* name) const;

	void SetUniform(const Uniform& uniform, int value) const;
	void SetUniform(const Uniform& uniform, float value) const;
	void SetUniform(const Uniform& uniform, const Vector3f& value) const;
	void SetUniform(const Uniform& uniform, const Vector4f& value) const;
	void SetUniform(const Uniform& uniform, const Matrix4& value) const;

	void Use();

private:

	void Create(const Renderer& renderer);
	void Link();

	glex* _glex;
	GLuint _spHandle;

};