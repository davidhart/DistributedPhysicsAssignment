// David Hart - 2011

#include "Application.h"
#include "MyWindow.h"
#include <iostream>

#include "Thread.h"

Application::Application()
{

}

void Application::Create(MyWindow& window)
{
	_renderer.Create(&window);

	_shapeBatch.Create(_renderer);

	_rotation = 0;

	_quads.SetSize(10000);

	Quad quad;

	quad._rotation = 0;

	for (int i = 0; i < 50; i++)
	{
		for (int j = 0; j < 50; j++)
		{
			quad._position = Vector2(i-25.0f, j-25.0f);
			_quads.SetQuad(i+j*50, quad);
		}
	}

	class TestStart : public ThreadStart
	{
	public:
		
		int sleepTime;

		unsigned Start()
		{
			Sleep(sleepTime);
			std::cout << "Thread Exited!" << std::endl;
			return 0;
		}
	};

	Thread threadA;
	Thread threadB;

	TestStart a; a.sleepTime = 500;
	TestStart b; b.sleepTime = 1000;

	threadA.Start(a);
	threadB.Start(b);

	threadB.Join();
	threadA.Join();

	_shapeBatch.AddQuadArray(&_quads);
}

void Application::Draw()
{
	_renderer.Clear();

	_shapeBatch.Draw(_renderer);
}

void Application::Update(float delta)
{
	for (unsigned int i = 0; i < _quads.GetSize(); i++)
	{
		Quad q = _quads.GetQuad(i);

		q._color = Color(rand()/(float)RAND_MAX, rand()/(float)RAND_MAX, rand()/(float)RAND_MAX);

		_quads.SetQuad(i, q);
	}

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