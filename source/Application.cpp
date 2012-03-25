// David Hart - 2011

#include "Application.h"
#include "MyWindow.h"
#include <iostream>

Application::Application() :
	_readState(_stateBuffers + 0),
	_writeState(_stateBuffers + 1)
{

}

void Application::Create(MyWindow& window)
{
	_renderer.Create(&window);

	_shapeBatch.Create(&_renderer);

	_rotation = 0;

	for (int i = 0; i < 25; i++)
	{
		for (int j = 0; j < 50; j++)
		{
			Quad& q = _readState->_quads[i + j * 25];

			q._position = Vector2(i-25.0f, j-25.0f);
			q._rotation = 0;
			q._color = Color(rand()/(float)RAND_MAX, rand()/(float)RAND_MAX, rand()/(float)RAND_MAX);
		}
	}

	_shapeBatch.AddQuadArray(&_quadBuffer);
	_quadBuffer.SetShapes(_readState->_quads, WorldState::NUM_QUADS);

	Triangle t;
	t._color = Color(1.0f, 0.0f, 0.0f);
	
	for (int i = 0; i < 25; i++)
	{
		for (int j = 0; j < 50; j++)
		{
			Triangle& t1 = _readState->_triangles[i + j * 25];
			Triangle& t2 = _readState->_triangles[i + j * 25 + 25 * 50];

			t2._points[0] = t1._points[0] = Vector2(1 + i - 1.0f, 0 + j - 25.0f);
			t1._points[1] = Vector2(0 + i - 1.0f, 0 + j - 25.0f);
			t2._points[2] = t1._points[2] = Vector2(0 + i - 1.0f, 1 + j - 25.0f);
			t2._points[1] = Vector2(1 + i - 1.0f, 1 + j - 25.0f);
		}
	}

	_shapeBatch.AddTriangleArray(&_triangleBuffer);
	_triangleBuffer.SetShapes(_readState->_triangles, WorldState::NUM_TRIANGLES);

	_physBossThread.SetReadState(_readState);
	_physBossThread.SetWriteState(_writeState);

	_physBossThread.BeginThreads();
	_physBossThread.BeginStep();
}

void Application::Draw()
{
	_renderer.Clear();
	_renderer.EnableDepthTest(false);

	_shapeBatch.Draw();
}

void Application::Update(double delta)
{
	_physBossThread.WaitForStepCompletion();

	std::swap(_writeState, _readState);
	_physBossThread.SetReadState(_readState);
	_physBossThread.SetWriteState(_writeState);

	_physBossThread.SetStepDelta(delta);
	_physBossThread.BeginStep();
	_quadBuffer.SetShapes(_readState->_quads, WorldState::NUM_QUADS);
	_triangleBuffer.SetShapes(_readState->_triangles, WorldState::NUM_TRIANGLES);
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