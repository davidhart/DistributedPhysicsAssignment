// David Hart - 2012

#include "Application.h"
#include "MyWindow.h"
#include <iostream>

Application::Application() :
	_drawState(NULL),
	_framesPerSecond(0),
	_viewZoom(8)
{

	for (int i = 0; i < NUM_CAMERA_ACTIONS; ++i)
	{
		_cameraState[i] = false;
	}
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

			q._position = Vector2f(i-25.0f, j-25.0f);
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

			t2._points[0] = t1._points[0] = Vector2f(1 + i - 1.0f, 0 + j - 25.0f);
			t1._points[1] = Vector2f(0 + i - 1.0f, 0 + j - 25.0f);
			t2._points[2] = t1._points[2] = Vector2f(0 + i - 1.0f, 1 + j - 25.0f);
			t2._points[1] = Vector2f(1 + i - 1.0f, 1 + j - 25.0f);
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
	UpdateCamera(delta);

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

void Application::UpdateCamera(double delta)
{
	Vector2f panDirection(0);
	bool cameraUpdate = false;
	if (_cameraState[CAMERA_PAN_LEFT])
	{
		panDirection.x(panDirection.x() - 1);
	}
	if (_cameraState[CAMERA_PAN_RIGHT])
	{
		panDirection.x(panDirection.x() + 1);
	}

	if (_cameraState[CAMERA_PAN_DOWN])
	{
		panDirection.y(panDirection.y() - 1);
	}
	if (_cameraState[CAMERA_PAN_UP])
	{
		panDirection.y(panDirection.y() + 1);
	}

	if (_cameraState[CAMERA_ZOOM_IN])
	{
		_viewZoom -= 3.0f * (float)delta;
	}
	if (_cameraState[CAMERA_ZOOM_OUT])
	{
		_viewZoom += 3.0f * (float)delta;
	}

	_viewTranslation += panDirection.normalize() * (float)delta * _viewZoom;
	UpdateViewMatrix();
}

void Application::Dispose()
{
	_shapeBatch.Dispose();
	_renderer.Dispose();

	_physBossThread.StopPhysics();
}

void Application::Resize(int width, int height)
{
	_aspect = width / (float)height;

	UpdateViewMatrix();
}

void Application::UpdateViewMatrix()
{
	Matrix4::Ortho2D(_view, 
					-_aspect*_viewZoom + _viewTranslation.x(), 
					_aspect*_viewZoom + _viewTranslation.x(), 
					_viewZoom + _viewTranslation.y(),
					-_viewZoom + _viewTranslation.y());

	_renderer.ViewMatrix(_view);
}

void Application::CameraKeyEvent(eCameraAction action, bool state)
{
	_cameraState[(int)action] = state;
}