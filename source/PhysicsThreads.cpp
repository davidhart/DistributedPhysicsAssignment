#include "PhysicsThreads.h"
#include "Application.h"

#include <iostream>


PhysicsWorkerThread::PhysicsWorkerThread() :
	_readState(NULL),
	_writeState(NULL),
	_delta(0),
	_threadId(0)
{
}

void PhysicsWorkerThread::SetReadState(WorldState* worldState)
{
	_readState = worldState;
}

void PhysicsWorkerThread::SetWriteState(WorldState* worldState)
{
	_writeState = worldState;
}

void PhysicsWorkerThread::SetStepDelta(double delta)
{
	_delta = delta;
}

void PhysicsWorkerThread::BeginStep()
{
	_physicsDone.Reset();
	_physicsBegin.Raise();
}

void PhysicsWorkerThread::WaitForStepCompletion()
{
	_physicsDone.Wait();
}

void PhysicsWorkerThread::SetThreadId(unsigned id)
{
	_threadId = id;
}

void PhysicsWorkerThread::SetNumThreads(unsigned numThreads)
{
	_numThreads = numThreads;
}

unsigned PhysicsWorkerThread::ThreadMain()
{
	_readState->i = 0;

	do
	{
		_physicsBegin.Wait();
		_physicsBegin.Reset();

		WorldState* readState = _readState;
		WorldState* writeState = _writeState;
		double delta = _delta;

		unsigned minIndex = GetStartIndex(WorldState::NUM_QUADS);
		unsigned maxIndex = GetEndIndex(WorldState::NUM_QUADS);

		for (unsigned int i = 0; i <= maxIndex; i++)
		{
			writeState->_quads[i]._position = readState->_quads[i]._position;
			float temp = pow(pow(pow(rand() / (float)RAND_MAX, 3), 3), 3);
			writeState->_quads[i]._rotation = readState->_quads[i]._rotation + 1 * (float)delta * 3 * temp;
			writeState->_quads[i]._color = readState->_quads[i]._color;
			writeState->_quads[i]._color = Color(rand()/(float)RAND_MAX, rand()/(float)RAND_MAX, rand()/(float)RAND_MAX);
		}

		minIndex = GetStartIndex(WorldState::NUM_TRIANGLES);
		maxIndex = GetEndIndex(WorldState::NUM_TRIANGLES);

		for (unsigned int i = minIndex; i <= maxIndex; i++)
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

unsigned PhysicsWorkerThread::GetStartIndex(unsigned count)
{
	unsigned countPerThread = count / _numThreads;

	return countPerThread * _threadId;
}

unsigned PhysicsWorkerThread::GetEndIndex(unsigned count)
{
	if (_threadId == _numThreads - 1)
		return count - 1;

	unsigned countPerThread = count / _numThreads;

	return countPerThread * _threadId + (countPerThread - 1);
}

PhysicsBossThread::PhysicsBossThread()
{
	SetThreadId(0);

	for (unsigned i = 0; i < 2; ++i)
	{
		_workers.push_back(new PhysicsWorkerThread());
		_workers[i]->SetThreadId(i+1);
	}

	SetNumThreads(_workers.size()+1);

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->SetNumThreads(_workers.size()+1);
	}

	std::cout << GetStartIndex(WorldState::NUM_QUADS) << " - " << GetEndIndex(WorldState::NUM_QUADS) << std::endl;

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		std::cout << _workers[i]->GetStartIndex(WorldState::NUM_QUADS) << " - " << _workers[i]->GetEndIndex(WorldState::NUM_QUADS) << std::endl;
	}
}

PhysicsBossThread::~PhysicsBossThread()
{
	for (unsigned i = 0; i < _workers.size(); ++i)
		delete _workers[i];
}

void PhysicsBossThread::SetReadState(WorldState* worldState)
{
	PhysicsWorkerThread::SetReadState(worldState);

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->SetReadState(worldState);
	}
}

void PhysicsBossThread::SetWriteState(WorldState* worldState)
{
	PhysicsWorkerThread::SetWriteState(worldState);

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->SetWriteState(worldState);
	}
}

void PhysicsBossThread::SetStepDelta(double delta)
{
	PhysicsWorkerThread::SetStepDelta(delta);

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->SetStepDelta(delta);
	}
}

void PhysicsBossThread::BeginThreads()
{
	Start();

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->Start();
	}
}

void PhysicsBossThread::BeginStep()
{
	PhysicsWorkerThread::BeginStep();

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->BeginStep();
	}
}

void PhysicsBossThread::WaitForStepCompletion()
{
	PhysicsWorkerThread::WaitForStepCompletion();

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->WaitForStepCompletion();
	}
}