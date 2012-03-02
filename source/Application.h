// David Hart - 2011
// 
// class Application
//   Application's responsibility is rendering and updating the objects
//   within the scene

#pragma once

#include "Renderer.h"
#include "ShapeBatch.h"

class MyWindow;

class Application
{

public:

	Application();
	void Create(MyWindow& window);
	void Dispose();
	void Draw();
	void Update(float delta);
	void Resize(int width, int height);

private:

	Matrix4 _view;
	Renderer _renderer;
	ShapeBatch _shapeBatch;
	QuadArray _quads;
	TriangleArray _triangles;
	float _rotation;
};