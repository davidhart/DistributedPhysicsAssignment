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

	void MouseMoved(const Vector2i& cursor);
	void LeftMouse(bool down);
	void RightMouse(bool down);

	void BeginSession();
	void JoinSession();
	void TerminateSession();

	void SetColorModeOwnership();
	void SetColorModeMass();
	void SetColorModeMotion();
	void SetColorModeProperty();

	void PlayPauseToggle();

	void ResetBlobby();

	void DrawHud();

	void SetGravity(double gravity);
	void SetFriction(double friction);
	void SetElasticity(double elasticity);

private:

	void Print(const std::string& string, int x, int y);
	void CreateHudFont(MyWindow& window);
	void DisposeHudFont();
	void UpdateCamera(double delta);
	void UpdateViewMatrix();
	void UpdatePeerBounds();

	void SendUserInputToWorld();
	Vector2d TranslateCursorToWorldCoords(const Vector2i& cursor);

	GLuint _fontList;

	Renderer _renderer;

	double _elapsed;
	unsigned _frameCount;

	unsigned _ticksPerSec;
	unsigned _framesPerSec;

	GameWorldThread _worldThread;

	World _world;

	Vector2f _viewTranslation;
	float _viewZoom;
	float _aspect;
	int _width;
	int _height;

	Matrix4 _view;

	static const float CAMERA_PAN_SPEED;
	static const float CAMERA_ZOOM_SPEED;

	static const int NUM_CAMERA_ACTIONS = 6;
	bool _cameraState[NUM_CAMERA_ACTIONS];

	struct
	{
		Vector2i _cursor;
		bool _leftButton;
		bool _rightButton;
		bool _changed;
	} _mouseState;
};