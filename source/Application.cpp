// David Hart - 2012

#include "Application.h"
#include "MyWindow.h"
#include <iostream>

const float Application::CAMERA_PAN_SPEED = 1.0f;
const float Application::CAMERA_ZOOM_SPEED = 3.0f;

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
	_world.Create(&_renderer, Vector2d(-100, 0), Vector2d(100, 100));
	
	Physics::PhysicsObject* o = _world.AddBlobbyObject();
	o->SetPosition(Vector2d(0, 40));
	
	for (int x = 0; x < 80; x++)
	{
		for (int y = 0; y < 10; y++)
		{
			Physics::BoxObject* b= _world.AddBox();
			b->SetPosition(Vector2d((x*1.2)-50, y*1.1+1));
			
			
			int m = rand() % 3;
			if (m == 0) b->SetMass(1);
			if (m == 1) b->SetMass(2);
			if (m == 2) b->SetMass(5);
			

			//b->SetVelocity(Vector2d(Util::RandRange(-1, 1), Util::RandRange(-1, 1)).normalize() * Util::RandRange(0, 80));
		}
	}
	
	/*
	Physics::BoxObject* b= _world.AddBox();
	b->SetPosition(Vector2d(0, 0.5));
	b->SetVelocity(Vector2d(1, 0));

	b= _world.AddBox();
	b->SetPosition(Vector2d(0, 7.5));
	b->SetVelocity(Vector2d(0, 0));*/
	
	/*
	for (int i = 0; i < 2; i++)
	{
			Physics::BoxObject* b= _world.AddBox();
			b->SetPosition(Vector2d(Util::RandRange(-20, 20), Util::RandRange(0, 20)));
			b->SetVelocity(Vector2d(0, 0));
			b->SetVelocity(Vector2d(Util::RandRange(-1, 1), Util::RandRange(-1, 1)).normalize() * Util::RandRange(0, 30));
	}*/

	_worldThread.SetWorld(&_world);
	_worldThread.BeginThreads();
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

	SendUserInputToWorld();

	_elapsed += delta;

	if (_elapsed > 1)
	{
		std::cout << "FPS: " << _framesPerSecond << std::endl;
		std::cout << "PPS: " << _worldThread.TicksPerSec() << std::endl;

		_framesPerSecond = 0;
		_worldThread.ResetTicksCounter();
		_elapsed = 0;
	}
}

void Application::UpdateCamera(double delta)
{
	Vector2f panDirection(0.0f);
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
		_viewZoom -= CAMERA_ZOOM_SPEED * (float)delta;
	}
	if (_cameraState[CAMERA_ZOOM_OUT])
	{
		_viewZoom += CAMERA_ZOOM_SPEED * (float)delta;
	}

	_viewTranslation += CAMERA_PAN_SPEED * panDirection.normalize() * (float)delta * _viewZoom;
	UpdateViewMatrix();
}

void Application::SendUserInputToWorld()
{
	if (_mouseState._changed)
	{
		_world.UpdateMouseInput(TranslateCursorToWorldCoords(_mouseState._cursor), _mouseState._leftButton, _mouseState._rightButton);

		_mouseState._changed = false;
	}
}

Vector2d Application::TranslateCursorToWorldCoords(const Vector2i& cursor)
{
	Vector2d position = Vector2d(cursor) / Vector2d(_width, _height) * Vector2d(_aspect * 2.0, 2.0) * (double)_viewZoom;
	
	position.y(position.y() * -1.0);
	
	position += Vector2d(_viewTranslation) + Vector2d(-_aspect, 1.0) * (double)_viewZoom;

	return position;
}

void Application::Dispose()
{
	_world.Dispose();
	_renderer.Dispose();

	_worldThread.StopPhysics();
}

void Application::Resize(int width, int height)
{
	_aspect = width / (float)height;
	_width = width;
	_height = height;

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

void Application::MouseMoved(const Vector2i& cursor)
{
	_mouseState._cursor = cursor;
	_mouseState._changed = true;
}

void Application::LeftMouse(bool down)
{
	_mouseState._leftButton = down;
	_mouseState._changed = true;
}

void Application::RightMouse(bool down)
{
	_mouseState._rightButton = down;
	_mouseState._changed = true;
}

void Application::BeginSession()
{
	_worldThread.CreateSession();
}

void Application::JoinSession()
{
	_worldThread.JoinSession();
}

void Application::TerminateSession()
{
	_worldThread.TerminateSession();
}

void Application::SetColorModeOwnership()
{
	_world.SetColorMode(COLOR_OWNERSHIP);
}

void Application::SetColorModeMass()
{
	_world.SetColorMode(COLOR_MASS);
}

void Application::SetColorModeMotion()
{
	_world.SetColorMode(COLOR_MOTION);
}

void Application::SetColorModeProperty()
{
	_world.SetColorMode(COLOR_PROPERTY);
}