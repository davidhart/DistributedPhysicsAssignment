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

class Application
{

public:

	Application();
	void Create(MyWindow& window);
	void Dispose();
	void Draw();
	void Update(double delta);
	void Resize(int width, int height);

private:

	Matrix4 _view;
	Renderer _renderer;
	ShapeBatch _shapeBatch;
	QuadArray _quadBuffer;
	TriangleArray _triangleBuffer;

	// 3 Copies of the world state for triple buffering
	static const int NUM_STATE_BUFFERS = 3;
	WorldState _stateBuffers[NUM_STATE_BUFFERS];

	double _elapsed;
	unsigned _framesPerSecond;

	WorldState* _drawState;

	PhysicsBossThread _physBossThread;
};