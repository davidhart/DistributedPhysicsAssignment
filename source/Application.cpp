// David Hart - 2012

#include "Application.h"
#include "MyWindow.h"
#include <iostream>

Application::Application() :
	_framesPerSecond(0),
	_viewZoom(16),
	_viewTranslation(0, 10)
{
	for (int i = 0; i < NUM_CAMERA_ACTIONS; ++i)
	{
		_cameraState[i] = false;
	}
}

void Application::Create(MyWindow& window)
{
	_renderer.Create(&window);
	_world.Create(&_renderer);

	for (int i = 0; i < 32000; i++)
	{
		Physics::BoxObject* b= _world.AddBox();
		b->SetPosition(Vector2d(0, 3));
		b->SetVelocity(Vector2d(Util::RandRange(-1, 1), Util::RandRange(-1, 1)).normalize() * Util::RandRange(0, 80));
	}

	_physBossThread.SetWorld(&_world);
	_physBossThread.BeginThreads();
}

void Application::Draw()
{
	_renderer.Clear();
	_renderer.EnableDepthTest(false);

	_world.Draw();

	_framesPerSecond++;
}

void Application::Update(double delta)
{
	UpdateCamera(delta);

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
	Vector2f panDirection(0.0f);
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
	_world.Dispose();
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