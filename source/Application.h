// David Hart - 2011
// 
// class Application
//   Application's responsibility is rendering and updating the objects
//   within the scene

#pragma once

#include "Renderer.h"
#include "PhysicsThreads.h"
#include "World.h"

class MyWindow;

enum eCameraAction
{
	CAMERA_PAN_LEFT = 0,
	CAMERA_PAN_RIGHT = 1,
	CAMERA_PAN_UP = 2,
	CAMERA_PAN_DOWN = 3,
	CAMERA_ZOOM_IN = 4,
	CAMERA_ZOOM_OUT = 5,
};

class Application
{

public:

	Application();
	void Create(MyWindow& window);
	void Dispose();
	void Draw();
	void Update(double delta);
	void Resize(int width, int height);

	void CameraKeyEvent(eCameraAction action, bool state);

private:

	void UpdateCamera(double delta);
	void UpdateViewMatrix();

	Renderer _renderer;

	double _elapsed;
	unsigned _framesPerSecond;

	PhysicsBossThread _physBossThread;

	World _world;

	Vector2f _viewTranslation;
	float _viewZoom;
	float _aspect;
	Matrix4 _view;

	static const int NUM_CAMERA_ACTIONS = 6;
	bool _cameraState[NUM_CAMERA_ACTIONS];
};