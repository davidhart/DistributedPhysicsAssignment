// David Hart - 2011

#include "Application.h"
#include "MyWindow.h"
#include <iostream>

Application::Application() :
	_drawState(NULL),
	_framesPerSecond(0)
{

}

void Application::Create(MyWindow& window)
{
	_physBossThread.SetReadState(_stateBuffers + 0);
	_physBossThread.SetWriteState(_stateBuffers + 1);
	_drawState = _stateBuffers + 0;

	_renderer.Create(&window);

	_shapeBatch.Create(&_renderer);

	for (int i = 0; i < 25; i++)
	{
		for (int j = 0; j < 50; j++)
		{
			Quad& q = _drawState->_quads[i + j * 25];

			q._position = Vector2(i-25.0f, j-25.0f);
			q._rotation = 0;
			q._color = Color(rand()/(float)RAND_MAX, rand()/(float)RAND_MAX, rand()/(float)RAND_MAX);
		}
	}

	_shapeBatch.AddQuadArray(&_quadBuffer);
	_quadBuffer.SetShapes(_drawState->_quads, WorldState::NUM_QUADS);

	Triangle t;
	t._color = Color(1.0f, 0.0f, 0.0f);
	
	for (int i = 0; i < 25; i++)
	{
		for (int j = 0; j < 50; j++)
		{
			Triangle& t1 = _drawState->_triangles[i + j * 25];
			Triangle& t2 = _drawState->_triangles[i + j * 25 + 25 * 50];

			t2._points[0] = t1._points[0] = Vector2(1 + i - 1.0f, 0 + j - 25.0f);
			t1._points[1] = Vector2(0 + i - 1.0f, 0 + j - 25.0f);
			t2._points[2] = t1._points[2] = Vector2(0 + i - 1.0f, 1 + j - 25.0f);
			t2._points[1] = Vector2(1 + i - 1.0f, 1 + j - 25.0f);
		}
	}

	_shapeBatch.AddTriangleArray(&_triangleBuffer);
	_triangleBuffer.SetShapes(_drawState->_triangles, WorldState::NUM_TRIANGLES);

	_drawState = _physBossThread.SwapDrawState(_stateBuffers + 2);
	_physBossThread.BeginThreads();
	//_physBossThread.BeginStep();
}

void Application::Draw()
{
	_renderer.Clear();
	_renderer.EnableDepthTest(false);

	_shapeBatch.Draw();

	_framesPerSecond++;
}

void Application::Update(double delta)
{
	_drawState = _physBossThread.SwapDrawState(_drawState);

	_quadBuffer.SetShapes(_drawState->_quads, WorldState::NUM_QUADS);
	_triangleBuffer.SetShapes(_drawState->_triangles, WorldState::NUM_TRIANGLES);

	_elapsed += delta;

	if (_elapsed > 1)
	{
		std::cout << "FPS: " << _framesPerSecond << std::endl;
		std::cout << "PPS: " << _physBossThread.TicksPerSec() << std::endl;

		_framesPerSecond = 0;
		_physBossThread.ResetTicksCounter();
		_elapsed = 0;
	}
}

void Application::Dispose()
{
	_shapeBatch.Dispose();
	_renderer.Dispose();

	_physBossThread.StopPhysics();
}

void Application::Resize(int width, int height)
{
	float aspect = width / (float)height;
	float scale = 8;
	Matrix4::Ortho2D(_view, -aspect*scale, aspect*scale, scale, -scale);
	_renderer.ViewMatrix(_view);
	// TODO: update view
}