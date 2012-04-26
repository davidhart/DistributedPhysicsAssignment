// David Hart - 2011
// 
// class MyWindow
//   MyWindow interfaces between the native system window and the application
//   config and application instance

#pragma once

#include "GXBase.h"
#include "Application.h"
#include "Config.h"

class MyWindow : public gxbase::GLWindowEx
{

public:

	MyWindow();
	void OnCreate();
	void OnDisplay();
	void OnIdle();
	void OnDestroy();
	void OnResize(int w, int h);
	void OnKeyboard(int key, bool down);
	void SetGXApp(gxbase::App* gxApp);
	void OnMouseMove(int x, int y);
	void OnMouseButton(gxbase::GLWindow::MouseButton button, bool down); 

private:

	Application _application;
	Config _config;
	gxbase::App* _gxApp;
	bool _loaded;
	bool _prevKeyStatePlus;
	bool _prevKeyStateMinus;
	Networking::System system;
};

