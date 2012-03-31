#include "PhysicsThreads.h"
#include "Application.h"

PhysicsWorkerThread::PhysicsWorkerThread() :
	_world(NULL),
	_haltPhysics(false),
	_delta(0),
	_threadId(0)
{
}

void PhysicsWorkerThread::SetWorld(World* world)
{
	_world = world;
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

	double delta = _delta;

	int minIndex = GetStartIndex(_world->GetNumObjects());
	int maxIndex = GetEndIndex(_world->GetNumObjects());

	for (int i = minIndex; i <= maxIndex; i++)
	{
		_world->UpdateObject(i, delta);
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

int PhysicsWorkerThread::GetStartIndex(unsigned count)
{
	if (count <= _numThreads)
	{
		if (_threadId >= count)
			return 0;

		return _threadId;
	}

	unsigned countPerThread = count / _numThreads;

	return countPerThread * _threadId;
}

int PhysicsWorkerThread::GetEndIndex(unsigned count)
{
	if (count <= _numThreads)
	{
		if (_threadId >= count)
			return -1;

		return _threadId;
	}

	if (_threadId == _numThreads - 1)
		return count - 1;

	unsigned countPerThread = count / _numThreads;

	return countPerThread * _threadId + (countPerThread - 1);
}

PhysicsBossThread::PhysicsBossThread() :
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

void PhysicsBossThread::SetWorld(World* world)
{
	PhysicsWorkerThread::SetWorld(world);

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->SetWorld(world);
	}
}

void PhysicsBossThread::BeginThreads()
{
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&startTime);

	Start();

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
	
	_world->SwapWriteState();
	
	// Stall the physics thread to prevent it from starving the render thread
	Sleep(0);

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