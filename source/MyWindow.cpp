// David Hart - 2012

#include "MyWindow.h"

using namespace gxbase;

MyWindow::MyWindow() :
	_gxApp(NULL),
	_loaded(false),
	_prevKeyStatePlus(false),
	_prevKeyStateMinus(false)
{
	SetTitle("Simulation and Concurrency");
	SetSize(800, 600);
	SetDoubleBuffer(true);
}

LARGE_INTEGER freq;
LARGE_INTEGER startTime;

void MyWindow::OnCreate()
{
	std::string file;

	if (_gxApp->ArgCount() == 1)
	{
		file = "default.cfg";
	}
	else if (_gxApp->ArgCount() == 2)
	{
		file = _gxApp->Arg(1);
	}
	else
	{
		MessageBoxA(GetSafeHwnd(), "Invalid Command Line Arguments usage:\nGraphicsACW.exe\nGraphicsACW.exe \"file\"", "Error", MB_OK);
		Close();
	}

	GLWindowEx::OnCreate();

	if (!_config.Read(file.c_str(), _application))
	{
		MessageBoxA(GetSafeHwnd(), (std::string("Invalid config file (") + file + std::string(")")).c_str(), "Error", MB_OK);
		Close();
	}
	else
	{
		_application.Create(*this);
		_loaded = true;
	}

	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&startTime);
}

void MyWindow::OnDisplay()
{
	LARGE_INTEGER endTime;
	QueryPerformanceCounter(&endTime);

	double delta = (double)(endTime.QuadPart-startTime.QuadPart)/freq.QuadPart;

	_application.Update(delta);
	_application.Draw();
	
	SwapBuffers();

	startTime = endTime;
}

void MyWindow::OnIdle()
{
	Redraw();
}

void MyWindow::OnDestroy()
{
	if (_loaded)
		_application.Dispose();

	GLWindowEx::OnDestroy();
}

void MyWindow::OnKeyboard(int key, bool down)
{
	if (VK_F9 == key && down)
	{
		bool full = !GetFullscreen();
		SetFullscreen(full);
	}
	else if (key == 'W' || key == VK_UP)
	{
		_application.CameraKeyEvent(CAMERA_PAN_UP, down);
	}
	else if (key == 'S' || key == VK_DOWN)
	{
		_application.CameraKeyEvent(CAMERA_PAN_DOWN, down);
	}
	else if (key == 'D' || key == VK_RIGHT)
	{
		_application.CameraKeyEvent(CAMERA_PAN_RIGHT, down);
	}
	else if (key == 'A' || key == VK_LEFT)
	{
		_application.CameraKeyEvent(CAMERA_PAN_LEFT, down);
	}
	else if (key == VK_ADD)
	{
		_application.CameraKeyEvent(CAMERA_ZOOM_IN, down);
	}
	else if (key == VK_SUBTRACT)
	{
		_application.CameraKeyEvent(CAMERA_ZOOM_OUT, down);
	}
	else if (VK_ESCAPE == key && down)
	{
		Close();
	}
	else if ('M' == key)
	{
		_application.BeginSession();
	}
	else if ('J' == key)
	{
		_application.JoinSession();
	}
	else if ('L' == key)
	{
		_application.TerminateSession();
	}
	else if ('1' == key)
	{
		_application.SetColorModeMass();
	}
	else if ('2' == key)
	{
		_application.SetColorModeMotion();
	}
	else if ('3' == key)
	{
		_application.SetColorModeOwnership();
	}
	else if ('4' == key)
	{
		_application.SetColorModeProperty();
	}
	else if ('B' == key)
	{
		_application.ResetBlobby();
	}
	else if ('P' == key && down)
	{
		_application.PlayPauseToggle();
	}
}

void MyWindow::OnMouseMove(int x, int y)
{
	_application.MouseMoved(Vector2i(x, Height() - y));
}

void MyWindow::OnMouseButton(gxbase::GLWindow::MouseButton button, bool down)
{
	if (GLWindow::MBLeft == button)
		_application.LeftMouse(down);

	if (GLWindow::MBRight == button)
		_application.RightMouse(down);
}

void MyWindow::OnResize(int w, int h)
{
	GLWindowEx::OnResize(w, h);
	_application.Resize(w, h);
}

void MyWindow::SetGXApp(gxbase::App* gxApp)
{
	_gxApp = gxApp;
}