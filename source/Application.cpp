// David Hart - 2011

#include "Application.h"
#include "MyWindow.h"

Application::Application()
{

}

void Application::Create(MyWindow& window)
{
	_renderer.Create(&window);
}

void Application::Draw()
{
	_renderer.Clear();


}

void Application::Update(float delta)
{

}

void Application::Dispose()
{

	_renderer.Dispose();
}


void Application::Resize(int width, int height)
{
	Matrix4 perspective;
	Matrix4::PerspectiveFov(perspective, 75, (float)width / height, 0.1f, 100);
	_renderer.ProjectionMatrix(perspective);
}