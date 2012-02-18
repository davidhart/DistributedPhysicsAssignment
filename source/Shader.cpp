// David Hart - 2011

#include "Shader.h"
#include "Renderer.h"
#include "Util.h"
#include <string>
#include <iostream>

Shader::Shader() :
	_type (0),
	_glex (NULL),
	_shHandle (0)
{
}

Shader::~Shader()
{
	assert(_shHandle == 0);
}

void Shader::CreateFromFile(const Renderer& renderer, const char* filename)
{
	std::string shaderSource;
	Util::ReadTextFileToString(filename, shaderSource);

#ifdef DEBUG
	std::cout << filename << std::endl; 
#endif
	Create(renderer, shaderSource.c_str());

}

void Shader::Create(const Renderer& renderer, const char* shadersource)
{
	if (_shHandle != 0)
		Dispose();

	assert(_shHandle == 0);

	_glex = renderer.GetEx();

	_shHandle = _glex->glCreateShader(_type);

	_glex->glShaderSource(_shHandle, 1, &shadersource, NULL);

	_glex->glCompileShader(_shHandle);


#ifdef DEBUG
	GLint loglength;
	_glex->glGetShaderiv(_shHandle, GL_INFO_LOG_LENGTH, &loglength);
	
	if (loglength != 0)
	{
		std::string log;
		log.resize(loglength);

		_glex->glGetShaderInfoLog(_shHandle, loglength, &loglength, &log[0]);

		std::cout << log << std::endl;
	}
#endif
}


void Shader::Dispose()
{
	assert(_shHandle != 0);

	_glex->glDeleteShader(_shHandle);

	_shHandle = 0;
	_glex = NULL;
}

void Shader::SetType(GLenum type)
{
	_type = type;
}

VertexShader::VertexShader()
{
	SetType(GL_VERTEX_SHADER);
}

FragmentShader::FragmentShader()
{
	SetType(GL_FRAGMENT_SHADER);
}