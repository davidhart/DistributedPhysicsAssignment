#include "PhysicsThreads.h"
#include "Application.h"

PhysicsStage::PhysicsStage()
{
	_begin.Reset();
	_end.Reset();
}

void PhysicsStage::Begin()
{
	_begin.Raise();
}

void PhysicsStage::Completed()
{
	_end.Raise();
}

void PhysicsStage::WaitForBegin()
{
	_begin.Wait();
	_begin.Reset();
}

void PhysicsStage::WaitForCompletion()
{
	_end.Wait();
	_end.Reset();
}

PhysicsWorkerThread::PhysicsWorkerThread() :
	_world(NULL),
	_haltPhysics(false),
	_delta(0),
	_threadId(0)
{
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
	Integrate();

	BroadPhase();

	SolveCollisions();
}

void PhysicsWorkerThread::Integrate()
{
	_integrationStage.WaitForBegin();

	double delta = _delta;

	int minIndex = GetStartIndex(_world->GetNumObjects());
	int maxIndex = GetEndIndex(_world->GetNumObjects());

	for (int i = minIndex; i <= maxIndex; i++)
	{
		_world->UpdateObject(i, delta);
	}

	_integrationStage.Completed();
}

void PhysicsWorkerThread::BroadPhase()
{
	_broadPhaseStage.WaitForBegin();

	int minIndex = GetStartIndex(_world->GetNumBucketsWide());
	int maxIndex = GetEndIndex(_world->GetNumBucketsWide());

	_world->BroadPhase(minIndex, maxIndex);

	_broadPhaseStage.Completed();
}

void PhysicsWorkerThread::SolveCollisions()
{
	_solveCollisionStage.WaitForBegin();

	int minIndex = GetStartIndex(_world->GetNumBucketsWide());
	int maxIndex = GetEndIndex(_world->GetNumBucketsWide());

	_world->SolveCollisions(minIndex, maxIndex);

	_solveCollisionStage.Completed();
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

GameWorldThread::GameWorldThread() :
	_tickCount(0),
	_shuttingDown(false),
	_state(STATE_STANDALONE),
	_networkController(NULL)
{
	SetThreadId(0);
	
	for (unsigned i = 0; i < 0; ++i)
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

GameWorldThread::~GameWorldThread()
{
	if (_networkController != NULL)
		delete _networkController;

	for (unsigned i = 0; i < _workers.size(); ++i)
		delete _workers[i];
}

void GameWorldThread::SetWorld(World* world)
{
	_world = world;

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->_world = world;
	}
}

void GameWorldThread::BeginThreads()
{
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&startTime);

	Start();

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->Start();
	}
}

// Bring the physics engine to a halt, this function will block until all active physics
// threads come to a halt
void GameWorldThread::StopPhysics()
{
	_shuttingDown = true;
	// Stop worker threads first because the BossThread will
	// not get stuck waiting to begin, whereas workers can
	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->Join();
	}

	
	Join();
}

void GameWorldThread::ExitWorkers()
{
	_haltPhysics = true;
	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->_haltPhysics = true;
	}
}

void GameWorldThread::PhysicsStep()
{
	_world->HandleUserInteraction();

	// TODO: timer class
	LARGE_INTEGER endTime;
	QueryPerformanceCounter(&endTime);
	
	double delta = (double)(endTime.QuadPart-startTime.QuadPart)/freq.QuadPart;
	
	SetStepDelta(delta);

	if (_shuttingDown)
	{
		ExitWorkers();
	}

	// Workers begin integration task
	BeginIntegration();
	PhysicsWorkerThread::Integrate();
	JoinIntegration();

	// Workers begin broadphase task
	BeginBroadphase();
	PhysicsWorkerThread::BroadPhase();
	JoinBroadphase();

	// Workers begin collision solve task
	BeginSolveCollisions();
	PhysicsWorkerThread::SolveCollisions();
	JoinSolveCollisions();

	_world->SwapWriteState();
	SanityCheckObjectsInBuckets();

	if (_state != STATE_STANDALONE)
	{
		_networkController->DoTick();
	}

	_tickCount++; // Record the step for performance measurement

	startTime = endTime;
}

void GameWorldThread::SanityCheckObjectsInBuckets()
{
	// Sanity test, total number of objects in buckets should equal total number of objects
	int test = 0;
	for (int y = _world->GetNumBucketsTall() - 1; y >= 0; --y)
	{
		for (int x = 0; x < _world->GetNumBucketsWide(); ++x)
		{
			test += _world->GetNumObjectsInBucket(x,y);
		}
	}

	if (test != _world->GetNumObjects())
	{
		std::cout << "Objects missing from buckets!!! " << test << std::endl;
	}
}

void GameWorldThread::ResetTicksCounter()
{
	_tickCount = 0;
}

unsigned GameWorldThread::TicksPerSec()
{
	return _tickCount;
}

void GameWorldThread::SetStepDelta(double delta)
{
	_delta = delta;

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->_delta = delta;
	}
}

void GameWorldThread::BeginIntegration()
{
	_integrationStage.Begin();

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->_integrationStage.Begin();
	}
}

void GameWorldThread::BeginBroadphase()
{
	_broadPhaseStage.Begin();

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->_broadPhaseStage.Begin();
	}
}

void GameWorldThread::BeginSolveCollisions()
{
	_solveCollisionStage.Begin();

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->_solveCollisionStage.Begin();
	}
}

void GameWorldThread::JoinIntegration()
{
	_integrationStage.WaitForCompletion();

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->_integrationStage.WaitForCompletion();
	}
}

void GameWorldThread::JoinBroadphase()
{
	_broadPhaseStage.WaitForCompletion();

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->_broadPhaseStage.WaitForCompletion();
	}
}

void GameWorldThread::JoinSolveCollisions()
{
	_solveCollisionStage.WaitForCompletion();

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->_solveCollisionStage.WaitForCompletion();
	}
}

void GameWorldThread::CreateSession()
{
	Threading::ScopedLock lock(_stateChangeMutex);

	if (_state == STATE_STANDALONE)
	{
		_networkController = new SessionMasterController(*this);
		_state = STATE_SESSIONMASTER;

		std::cout << "Creating session" << std::endl;
	}
}

void GameWorldThread::JoinSession()
{
	Threading::ScopedLock lock(_stateChangeMutex);

	if (_state == STATE_STANDALONE)
	{
		_networkController = new WorkerController(*this);
		_state = STATE_SESSIONWORKER;

		std::cout << "Attempting to find session" << std::endl;
	}
}

void GameWorldThread::TerminateSession()
{
	Threading::ScopedLock lock(_stateChangeMutex);

	if (_state != STATE_STANDALONE)
	{

		delete _networkController;
		_networkController = NULL;
		_state = STATE_STANDALONE;

		std::cout << "Session terminated" << std::endl;
	}
}