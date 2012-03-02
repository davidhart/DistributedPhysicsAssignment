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

	_quads.SetSize(25*50);

	Quad quad;

	quad._rotation = 0;

	for (int i = 0; i < 25; i++)
	{
		for (int j = 0; j < 50; j++)
		{
			quad._position = Vector2(i-25.0f, j-25.0f);
			_quads.SetShape(i+j*25, quad);
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

	_shapeBatch.AddQuadArray(&_quads);

	_triangles.SetSize(25 * 50 * 2);

	Triangle t;
	t._color = Color(1.0f, 0.0f, 0.0f);
	
	for (int i = 0; i < 25; i++)
	{
		for (int j = 0; j < 50; j++)
		{
			t._points[0] = Vector2(1 + i - 1.0f, 0 + j - 25.0f);
			t._points[1] = Vector2(0 + i - 1.0f, 0 + j - 25.0f);
			t._points[2] = Vector2(0 + i - 1.0f, 1 + j - 25.0f);

			_triangles.SetShape(i + j * 25, t);

			t._points[1] = Vector2(1 + i - 1.0f, 1 + j - 25.0f);

			_triangles.SetShape(i + j * 25 + 25 * 50, t);
		}
	}

	_shapeBatch.AddTriangleArray(&_triangles);

	Thread threadA;
	Thread threadB;

	TestStart a; a.sleepTime = 500;
	TestStart b; b.sleepTime = 1000;

	threadA.Start(a);
	threadB.Start(b);

	threadB.Join();
	threadA.Join();
}

void Application::Draw()
{
	_renderer.Clear();
	_renderer.EnableDepthTest(false);

	_shapeBatch.Draw(_renderer);
}

void Application::Update(float delta)
{
	for (unsigned int i = 0; i < _quads.GetSize(); i++)
	{
		Quad q = _quads.GetShape(i);

		q._color = Color(rand()/(float)RAND_MAX, rand()/(float)RAND_MAX, rand()/(float)RAND_MAX);

		_quads.SetShape(i, q);
	}

	for (unsigned int i = 0; i < _triangles.GetSize(); i++)
	{
		Triangle t = _triangles.GetShape(i);

		t._color = Color(rand()/(float)RAND_MAX, rand()/(float)RAND_MAX, rand()/(float)RAND_MAX);

		_triangles.SetShape(i, t);
	}


	_rotation = fmod(delta + _rotation, PI*2);
	Quad q = _quads.GetShape(0);
	q._rotation = _rotation;
	_quads.SetShape(0, q);
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