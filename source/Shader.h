// David Hart - 2011
//
// class Shader
//    Shader is a wrapper around the opengl Shader object, Vertex and 
//    FragmentShader represent GL_VERTEX_SHADER and GL_FRAGMENT_SHADER
//    objects respectively.
//


#pragma once

#include "Uncopyable.h"
#include "GXBase.h"

class Renderer;

class Shader : public Uncopyable
{

	friend class ShaderProgram;

public:

	virtual ~Shader();

	void CreateFromFile(const Renderer& renderer, const char* filename);
	void Create(const Renderer& renderer, const char* shadersource);
	void Dispose();

protected:
	
	void SetType(GLenum type);
	Shader();

private:
	
	GLenum _type;
	glex* _glex;
	GLuint _shHandle;

};

class VertexShader : public Shader
{

public:

	VertexShader();

};

class FragmentShader : public Shader
{

public:

	FragmentShader();

};

enum eGeometryInput
{
	GI_POINTS = GL_POINTS,
	GI_TRIANGLES = GL_TRIANGLES,
};

enum eGeometryOutput
{
	GO_POINTS = GL_POINTS,
	GO_TRIANGLES = GL_TRIANGLES,
};

class GeometryShader : public Shader
{

public:

	GeometryShader();

};
