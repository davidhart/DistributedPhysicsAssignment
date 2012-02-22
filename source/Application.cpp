// David Hart - 2011

#include "Application.h"
#include "MyWindow.h"
#include <iostream>

Application::Application()
{

}

void Application::Create(MyWindow& window)
{
	_renderer.Create(&window);

	_shapeBatch.Create(_renderer);

	_rotation = 0;

	_quads.SetColor(Vector4(0.5f, 1.0f, 0.5f, 1.0f));
	_quads.SetSize(2);

	Quad quad;
	quad._position = Vector2(1.0f, 1.0f);
	quad._rotation = 0;

	_quads.SetQuad(0, quad);

	_shapeBatch.AddQuadArray(&_quads);
}

void Application::Draw()
{
	_renderer.Clear();

	_shapeBatch.Draw(_renderer);
}

void Application::Update(float delta)
{
	_rotation = fmod(delta + _rotation, PI*2);
	Quad q = _quads.GetQuad(0);
	q._rotation = _rotation;
	_quads.SetQuad(0, q);
}

void Application::Dispose()
{
	_shapeBatch.Dispose();
	_renderer.Dispose();
}


void Application::Resize(int width, int height)
{
	float aspect = width / (float)height;
	float scale = 8;
	Matrix4::Ortho2D(_view, -aspect*scale, aspect*scale, scale, -scale);
	_renderer.ViewMatrix(_view);
	// TODO: update view
}