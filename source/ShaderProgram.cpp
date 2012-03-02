// David Hart - 2011

#include "ShaderProgram.h"
#include "Renderer.h"
#include <string>
#include <iostream>

Uniform::Uniform() :
	_location(-1)
{
}

Uniform::Uniform(GLint location) :
	_location(location)
{
}

ShaderProgram::ShaderProgram() : 
	_glex (NULL),
	_spHandle (0)
{
}

ShaderProgram::~ShaderProgram()
{
	assert(_spHandle == 0);
}

void ShaderProgram::Create(const Renderer& renderer, const VertexShader& vertexShader, const FragmentShader& fragmentShader)
{
	Create(renderer);

	_glex->glAttachShader(_spHandle, vertexShader._shHandle);
	_glex->glAttachShader(_spHandle, fragmentShader._shHandle);

	Link();
}

void ShaderProgram::Create(const Renderer& renderer, const VertexShader& vertexShader, const FragmentShader& fragmentShader, const GeometryShader& geometryShader)
{
	Create(renderer);

	_glex->glAttachShader(_spHandle, vertexShader._shHandle);
	_glex->glAttachShader(_spHandle, fragmentShader._shHandle);
	_glex->glAttachShader(_spHandle, geometryShader._shHandle);

	Link();
}

void ShaderProgram::Create(const Renderer& renderer)
{
	if (_spHandle != 0)
		Dispose();

	assert(_spHandle == 0);

	_glex = renderer.GetEx();

	_spHandle = _glex->glCreateProgram();
}

void ShaderProgram::Link()
{
	_glex->glLinkProgram(_spHandle);

#ifdef DEBUG
	GLint loglength;
	_glex->glGetProgramiv(_spHandle, GL_INFO_LOG_LENGTH, &loglength);

	if (loglength != 0)
	{
		std::string log;
		log.resize(loglength);

		_glex->glGetProgramInfoLog(_spHandle, loglength, &loglength, &log[0]);

		std::cout << log << std::endl;
	}
#endif
}

void ShaderProgram::Dispose()
{
	assert(_spHandle != 0);

	_glex->glDeleteProgram(_spHandle);
	_spHandle = 0;
	_glex = NULL;
}

void ShaderProgram::Use()
{
	_glex->glUseProgram(_spHandle);
}

int ShaderProgram::GetAttributeIndex(const char* attribute) const
{
	return _glex->glGetAttribLocation(_spHandle, attribute);
}

Uniform ShaderProgram::GetUniform(const char* name) const
{
	return Uniform(_glex->glGetUniformLocation(_spHandle, name));
}

void ShaderProgram::SetUniform(const Uniform& uniform, const Matrix4& value) const
{
	if (uniform._location >= 0)
	{
		_glex->glUniformMatrix4fv(uniform._location, 1, false, (const float*)&value);
	}
}

void ShaderProgram::SetUniform(const Uniform& uniform, const Vector4& value) const
{
	if (uniform._location >= 0)
	{
		_glex->glUniform4fv(uniform._location, 1, (const float*)&value);
	}
}

void ShaderProgram::SetUniform(const Uniform& uniform, const Vector3& value) const
{
	if (uniform._location >= 0)
	{
		_glex->glUniform3fv(uniform._location, 1, (const float*)&value);
	}
}
void ShaderProgram::SetUniform(const Uniform& uniform, float value) const
{
	if (uniform._location >= 0)
	{
		_glex->glUniform1f(uniform._location, value);
	}
}

void ShaderProgram::SetUniform(const Uniform& uniform, int value) const
{
	if (uniform._location >= 0)
	{
		_glex->glUniform1i(uniform._location, value);
	}
}
