// David Hart - 2012

#include "Application.h"
#include "MyWindow.h"
#include <iostream>
#include <sstream>

const float Application::CAMERA_PAN_SPEED = 1.0f;
const float Application::CAMERA_ZOOM_SPEED = 3.0f;

Application::Application() :
	_frameCount(0),
	_ticksPerSec(0),
	_framesPerSec(0),
	_viewZoom(32),
	_viewTranslation(0, 30)
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
	
	for (int x = 0; x < 80; x++)
	{
		for (int y = 0; y < 10; y++)
		{
			Physics::BoxObject* b= _world.AddBox();
			b->SetPosition(Vector2d((x*1.2)-40*1.2, y*1.1+1));
			
			int m = rand() % 3;
			if (m == 0) b->SetMass(1);
			if (m == 1) b->SetMass(2);
			if (m == 2) b->SetMass(5);
		}
	}

	_worldThread.SetWorld(&_world);
	_worldThread.BeginThreads();


	CreateHudFont(window);
}

void Application::CreateHudFont(MyWindow& window)
{
	// Using simple deprecated font code in the interest of time
	HFONT   font;                                                                           
	HFONT   oldfont;                                                                        
	_fontList = glGenLists(256);                                                        
	font = CreateFont(-10, 0, 0, 0, FALSE, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE|DEFAULT_PITCH, "System");                                      
	HDC hDC = GetWindowDC(window.GetSafeHwnd());
	oldfont = (HFONT)SelectObject(hDC, font);          
	wglUseFontBitmaps(hDC, 32, 96, _fontList);                         
	SelectObject(hDC, oldfont);                                            
	DeleteObject(font);
}

void Application::DisposeHudFont()
{
	glDeleteLists(_fontList, 256);
}

void Application::Print(const std::string& string, int x, int y)
{
	float xPos = x / (float)_width * 2 - 1;
	float yPos = y / (float)_height * -2 + 1;

	glRasterPos2f(xPos, yPos);

	glPushAttrib(GL_LIST_BIT);                                              
	glListBase(_fontList - 32);                                                               
	glCallLists(string.size()+1, GL_UNSIGNED_BYTE, string.c_str());
	glPopAttrib();  
}

void Application::Draw()
{
	_renderer.Clear();
	_renderer.EnableDepthTest(false);

	_world.Draw();

	Print("Simulation and Concurrency - David Hart", 0, 13);

	std::stringstream ss;
	ss << "Render framerate: " << _framesPerSec << "   " << "Physics framerate: " << _ticksPerSec;

	Print(ss.str(), 0, 13*2);

	ss = std::stringstream();
	ss << "Objects: " << _world.GetNumObjects();

	Print(ss.str(), 0, 13*3);

	_frameCount++;
}

void Application::Update(double delta)
{
	UpdateCamera(delta);

	SendUserInputToWorld();

	_elapsed += delta;

	if (_elapsed > 1)
	{
		_framesPerSec = _frameCount;
		_ticksPerSec = _worldThread.TicksPerSec();

		_frameCount = 0;
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

	DisposeHudFont();
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

void Application::ResetBlobby()
{
	_world.ResetBlobbyPressed();
}