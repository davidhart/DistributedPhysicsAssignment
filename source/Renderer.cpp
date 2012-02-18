// David Hart - 2011

#include "Renderer.h"
#include "VertexBinding.h"
#include "ShaderProgram.h"
#include <cassert>

Renderer::Renderer() :
	_glex (NULL)
{
}

Renderer::~Renderer()
{
	assert(_glex == NULL);
}

void Renderer::Create(glex* glex)
{
	assert(glex != NULL);

	_glex = glex;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearStencil(0);
}

glex* Renderer::GetEx() const
{
	assert(_glex != NULL);

	return _glex;
}

void Renderer::Dispose()
{
	assert(_glex != NULL);

	_glex = NULL;
}

void Renderer::Draw(VertexBinding& binding, ePrimitive primitive, unsigned int offset, unsigned int indices) const
{
	binding.Bind();

	if (binding._hasIndices)
	{
		glDrawElements(primitive, indices, binding._indicesType, (void*)offset);
	}
	else
	{
		glDrawArrays(primitive, offset, indices);
	}

	binding.Unbind();
}

void Renderer::DrawInstances(VertexBinding& binding, ePrimitive primitive, unsigned int offset, unsigned int indices, unsigned int instances) const
{
	binding.Bind();

	_glex->glDrawElementsInstanced(primitive, indices, binding._indicesType, (void*)offset, instances);

	binding.Unbind();
}

void Renderer::ProjectionMatrix(const Matrix4& projection)
{
	_projection = projection;
}

void Renderer::ViewMatrix(const Matrix4& view)
{
	_view = view;
}

void Renderer::ClipPlane(const Vector4& clipPlane)
{
	_clipPlane = clipPlane;
}

void Renderer::SetAmbient(const Vector3& ambient)
{
	_ambient = ambient;
}

void Renderer::UpdateStandardUniforms(const ShaderProgram& shaderprogram, const StandardUniformBlock& uniforms) const
{
	// TODO: replace these
	shaderprogram.SetUniform(uniforms.View, _view);
	shaderprogram.SetUniform(uniforms.Projection, _projection);
	shaderprogram.SetUniform(uniforms.ClipPlane, _clipPlane);
	shaderprogram.SetUniform(uniforms.Ambient, _ambient);
}

void Renderer::GetStandardUniforms(const ShaderProgram& shaderProgram, StandardUniformBlock & uniforms) const
{
	uniforms.Model = shaderProgram.GetUniform("model");
	uniforms.View = shaderProgram.GetUniform("view");
	uniforms.Projection = shaderProgram.GetUniform("projection");
	uniforms.ClipPlane = shaderProgram.GetUniform("clipPlane");

	uniforms.Ambient = shaderProgram.GetUniform("ambient");
}

void Renderer::EnableCullFace(bool enable) const
{
	if (enable)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

void Renderer::CullFace(eCullFace face) const
{
	glCullFace(face);
}

void Renderer::EnableBlend(bool enable) const
{
	if (enable)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);
}

void Renderer::BlendMode(eBlendMode mode) const
{
	if (mode == BLEND_ADDITIVE)
	{
		glBlendFunc(GL_SRC_COLOR, GL_ONE);
	}
	else if (mode == BLEND_ALPHA)
	{
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}

void Renderer::EnableDepthTest(bool enable) const
{
	if (enable)
		glDepthFunc(GL_LESS);
	else
		glDepthFunc(GL_ALWAYS);
}

void Renderer::EnableDepthWrite(bool enable) const
{
	if (enable)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);
}


void Renderer::EnableColorWrite(bool enable) const
{
	if (enable)
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	else
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
}

void Renderer::EnableStencilTest(bool enable) const
{
	if (enable)
		glEnable(GL_STENCIL_TEST);
	else
		glDisable(GL_STENCIL_TEST);
}

void Renderer::StencilOp(eStencilOp stencilFail, eStencilOp stencilPassDepthFail, eStencilOp stencilDepthPass) const
{
	glStencilOp(stencilFail, stencilPassDepthFail, stencilDepthPass);
}

void Renderer::StencilTest(eStencilFunc func, int ref) const
{
	glStencilFunc(func, ref, 0xFFFFFFFF);
}

void Renderer::Clear() const
{
	// TODO: parameterise
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}