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
	Quad& quad = _quads.GetQuad(0);
	quad._position = Vector2(-0.5f, -0.5f);
	quad._rotation = 0;

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
	_quads.GetQuad(0)._rotation = _rotation;
}

void Application::Dispose()
{
	_quads.Dispose();

	_shapeBatch.Dispose();

	_renderer.Dispose();
}


void Application::Resize(int width, int height)
{
	// TODO: update view
}