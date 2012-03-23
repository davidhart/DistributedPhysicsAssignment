#include "PhysicsBossThread.h"
#include "Application.h"

#include <iostream>

unsigned PhysicsBossThread::ThreadMain()
{
	_readState->i = 0;

	do
	{
		_physicsBegin.Wait();
		_physicsBegin.Reset();

		WorldState* readState = _readState;
		WorldState* writeState = _writeState;
		double delta = _delta;

		for (unsigned int i = 0; i < WorldState::NUM_QUADS; i++)
		{
			writeState->_quads[i]._position = readState->_quads[i]._position;
			writeState->_quads[i]._rotation = readState->_quads[i]._rotation + 1 * (float)delta;
			writeState->_quads[i]._color = readState->_quads[i]._color;
			writeState->_quads[i]._color = Color(rand()/(float)RAND_MAX, rand()/(float)RAND_MAX, rand()/(float)RAND_MAX);
		}

		for (unsigned int i = 0; i < WorldState::NUM_TRIANGLES; i++)
		{
			writeState->_triangles[i]._points[0] = readState->_triangles[i]._points[0];
			writeState->_triangles[i]._points[1] = readState->_triangles[i]._points[1];
			writeState->_triangles[i]._points[2] = readState->_triangles[i]._points[2];
			writeState->_triangles[i]._color = Color(rand()/(float)RAND_MAX, rand()/(float)RAND_MAX, rand()/(float)RAND_MAX);
		}

		_writeState->i = _readState->i + 1;

		_physicsDone.Raise();
	} while(true);
}