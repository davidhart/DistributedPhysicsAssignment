#include "PhysicsThreads.h"
#include "Application.h"

PhysicsWorkerThread::PhysicsWorkerThread() :
	_readState(NULL),
	_writeState(NULL),
	_haltPhysics(false),
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

void PhysicsWorkerThread::BeginStep(double delta)
{
	_delta = delta;
	_physicsDone.Reset();
	_physicsBegin.Raise();
}

void PhysicsWorkerThread::WaitForStepCompletion()
{
	// If we are shutting down there is a chance a worker thread already exited, 
	// in which case we should not wait for it
	if (!_haltPhysics && IsRunning())
		_physicsDone.Wait();
}

void PhysicsWorkerThread::StopPhysics()
{
	_haltPhysics = true;

	Join();
}

void PhysicsWorkerThread::SetThreadId(unsigned id)
{
	_threadId = id;
}

void PhysicsWorkerThread::SetNumThreads(unsigned numThreads)
{
	_numThreads = numThreads;
}

void PhysicsWorkerThread::PhysicsStep()
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
		writeState->_quads[i]._rotation = readState->_quads[i]._rotation + 1 * (float)delta * 0.5f;
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

	_physicsDone.Raise();
}

unsigned PhysicsWorkerThread::ThreadMain()
{
	while(!_haltPhysics)
	{
		PhysicsStep();
	}

	return 0;
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

PhysicsBossThread::PhysicsBossThread() :
	_freeState(NULL),
	_tickCount(0)
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

void PhysicsBossThread::BeginThreads()
{
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&startTime);

	Start();

	_readState->i = 0;

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->Start();
	}
}

void PhysicsBossThread::BeginStep(double delta)
{
	PhysicsWorkerThread::BeginStep(delta);

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->BeginStep(delta);
	}
}

WorldState* PhysicsBossThread::SwapDrawState(WorldState* oldDrawState)
{
	_stateSwapMutex.Enter();

	if (oldDrawState != _readState)
	{
		_freeState = oldDrawState;
	}

	WorldState* readState = _readState;

	_stateSwapMutex.Exit();

	return readState;
}

void PhysicsBossThread::StopPhysics()
{
	// Stop worker threads first because the BossThread will
	// not get stuck waiting to begin, whereas workers can
	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->StopPhysics();
	}

	PhysicsWorkerThread::StopPhysics();
}


void PhysicsBossThread::WaitForStepCompletion()
{
	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->WaitForStepCompletion();
	}
	
	PhysicsWorkerThread::WaitForStepCompletion();
}

void PhysicsBossThread::PhysicsStep()
{
	// TODO: timer class
	LARGE_INTEGER endTime;
	DWORD oldAff = SetThreadAffinityMask(GetCurrentThread(), 1);
	QueryPerformanceCounter(&endTime);
	
	double delta = (double)(endTime.QuadPart-startTime.QuadPart)/freq.QuadPart;
	
	// Wake up the workers
	BeginStep(delta);

	// Complete the current step on all threads including this one
	PhysicsWorkerThread::PhysicsStep();

	// Synchronise with workers
	WaitForStepCompletion();
	
	// ---- Critical Section Begin
	_stateSwapMutex.Enter();

	// The state we just wrote is now the readable state
	SetReadState(_writeState);

	// Write to the invalid state, which is either the one just wrote
	// or one that the renderer recently finished with
	SetWriteState(_freeState);

	// The previous readState will be invalid after the next step and  
	// will be written to at the next oppertunity
	_freeState = _readState;

	_stateSwapMutex.Exit();
	// ---- Critical Section End

	_tickCount++; // Record the step for performance measurement

	startTime = endTime;
}

void PhysicsBossThread::ResetTicksCounter()
{
	_tickCount = 0;
}

unsigned PhysicsBossThread::TicksPerSec()
{
	return _tickCount;
}