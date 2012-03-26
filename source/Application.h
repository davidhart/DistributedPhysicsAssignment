// David Hart - 2011
// 
// class Application
//   Application's responsibility is rendering and updating the objects
//   within the scene

#pragma once

#include "Renderer.h"
#include "ShapeBatch.h"
#include "PhysicsThreads.h"

class MyWindow;

struct WorldState
{

public:

	int i;

	static const int NUM_QUADS = 25*50;
	static const int NUM_TRIANGLES = 25 * 50 * 2;

	Quad _quads[NUM_QUADS];
	Triangle _triangles[NUM_TRIANGLES];

};

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
	ShapeBatch _shapeBatch;
	QuadArray _quadBuffer;
	TriangleArray _triangleBuffer;

	// 3 Copies of the world state for triple buffering
	static const int NUM_STATE_BUFFERS = 3;
	WorldState _stateBuffers[NUM_STATE_BUFFERS];
	WorldState* _drawState;

	double _elapsed;
	unsigned _framesPerSecond;

	PhysicsBossThread _physBossThread;

	Vector2f _viewTranslation;
	float _viewZoom;
	float _aspect;
	Matrix4 _view;

	static const int NUM_CAMERA_ACTIONS = 6;
	bool _cameraState[NUM_CAMERA_ACTIONS];
};