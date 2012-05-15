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
	_threadId(0),
	_peerId(0),
	_numPeers(1)
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

void PhysicsWorkerThread::SetPeerId(unsigned id)
{
	_peerId = id;
}

void PhysicsWorkerThread::SetNumPeers(unsigned numPeers)
{
	_numPeers = numPeers;
}

void PhysicsWorkerThread::PhysicsStep()
{
	Integrate();

	BroadPhase();

	DetectCollisions();

	SolveCollisions();
}

void PhysicsWorkerThread::Integrate()
{
	_integrationStage.WaitForBegin();

	double delta = _delta;

	// Integrate the objects we are responsible for
	int minIndex = GetStartIndexForId(_threadId, _numThreads, _world->GetNumObjects());
	int maxIndex = GetEndIndexForId(_threadId, _numThreads, _world->GetNumObjects());

	for (int i = minIndex; i <= maxIndex; i++)
	{
		_world->UpdateObject(i, delta);
	}

	_integrationStage.Completed();
}

void PhysicsWorkerThread::BroadPhase()
{
	_broadPhaseStage.WaitForBegin();

	// The broadphase for all buckets must be done by all clients
	int minIndex = GetStartIndexForId(_threadId, _numThreads, _world->GetNumBucketsWide());
	int maxIndex = GetEndIndexForId(_threadId, _numThreads, _world->GetNumBucketsWide());

	_world->BroadPhase(minIndex, maxIndex);

	_broadPhaseStage.Completed();
}

void PhysicsWorkerThread::DetectCollisions()
{
	_detectCollisionStage.WaitForBegin();

	// Do narrowphase in the area this thread & peer is responsible for
	/*
	int minIndex = GetPeerStartIndex(_world->GetNumBucketsWide());
	int maxIndex = GetPeerEndIndex(_world->GetNumBucketsWide());
	*/

	int minIndex = GetStartIndexForId(_threadId, _numThreads, _world->GetNumBucketsWide());
	int maxIndex = GetEndIndexForId(_threadId, _numThreads, _world->GetNumBucketsWide());

	_world->DetectCollisions(minIndex, maxIndex);

	_detectCollisionStage.Completed();
}

void PhysicsWorkerThread::SolveCollisions()
{
	_solveCollisionStage.WaitForBegin();

	int minIndex = GetStartIndexForId(_threadId, _numThreads, _world->GetNumObjects());
	int maxIndex = GetEndIndexForId(_threadId, _numThreads, _world->GetNumObjects());

	for (int i = minIndex; i <= maxIndex; ++i)
	{
		Physics::PhysicsObject* object = _world->GetObject(i);

		if (object->GetOwnerId() == _peerId)
		{
			object->SolveContacts(*_world);
		}

		object->UpdateShape(*_world);
	}

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

int PhysicsWorkerThread::GetStartIndexForId(unsigned id, unsigned numIds, unsigned count)
{
	if (count <= numIds)
	{
		if (id >= count)
			return 0;

		return id;
	}

	unsigned countPerId = count / numIds;

	return countPerId * id;
}

int PhysicsWorkerThread::GetEndIndexForId(unsigned id, unsigned numIds, unsigned count)
{
	if (count <= numIds)
	{
		if (id >= count)
			return -1;

		return id;
	}

	if (id == numIds - 1)
		return count - 1;

	unsigned countPerId = count / numIds;

	return countPerId * id + (countPerId - 1);
}

int PhysicsWorkerThread::GetPeerStartIndex(unsigned count)
{
	int peerStart = GetStartIndexForId(_peerId, _numPeers, count);
	int peerEnd = GetEndIndexForId(_peerId, _numPeers, count) + 1;

	return peerStart + GetStartIndexForId(_threadId, _numThreads, peerEnd - peerStart);
}

int PhysicsWorkerThread::GetPeerEndIndex(unsigned count)
{
	int peerStart = GetStartIndexForId(_peerId, _numPeers, count);
	int peerEnd = GetEndIndexForId(_peerId, _numPeers, count) + 1;

	return peerStart + GetEndIndexForId(_threadId, _numThreads, peerEnd - peerStart);
}

GameWorldThread::GameWorldThread() :
	_tickCount(0),
	_shuttingDown(false),
	_state(STATE_STANDALONE),
	_networkController(NULL)
{
	SetThreadId(0);
	
	for (unsigned i = 0; i < 1; ++i)
	{
		_workers.push_back(new PhysicsWorkerThread());
		_workers[i]->SetThreadId(i+1);
	}

	SetNumThreads(_workers.size()+1);

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->SetNumThreads(_workers.size()+1);

	}

	int minIndex = GetStartIndexForId(0, _numThreads, _world->GetNumBucketsWide());
	int maxIndex = GetEndIndexForId(0, _numThreads, _world->GetNumBucketsWide());

	std::cout << minIndex << " " << maxIndex << std::endl;

	minIndex = GetStartIndexForId(1, _numThreads, _world->GetNumBucketsWide());
	maxIndex = GetEndIndexForId(1, _numThreads, _world->GetNumBucketsWide());

	std::cout << minIndex << " " << maxIndex << std::endl;

	minIndex = GetStartIndexForId(2, _numThreads, _world->GetNumBucketsWide());
	maxIndex = GetEndIndexForId(2, _numThreads, _world->GetNumBucketsWide());

	std::cout << minIndex << " " << maxIndex << std::endl;

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
	_timer.Start();

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
	if (_shuttingDown)
	{
		ExitWorkers();
	}

	
	double delta = _timer.GetTime();
	_timer.Start();

	_world->HandleUserInteraction();
	
	if (delta < 0) delta = 0;
	SetStepDelta(delta);

	// Workers begin integration task
	BeginIntegration();
	PhysicsWorkerThread::Integrate();
	JoinIntegration();

	// Workers begin broadphase task
	BeginBroadphase();
	PhysicsWorkerThread::BroadPhase();
	JoinBroadphase();

	// Workers begin collision detect task
	BeginDetectCollisions();
	PhysicsWorkerThread::DetectCollisions();
	JoinDetectCollisions();

	// Workers begin collision solve task
	BeginSolveCollisions();
	PhysicsWorkerThread::SolveCollisions();
	JoinSolveCollisions();

	if (_state != STATE_STANDALONE)
	{
		_networkController->ExchangeState();
	}

	_world->SwapWriteState();

	_tickCount++; // Record the step for performance measurement
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

double GameWorldThread::GetStepDelta()
{
	return _delta;
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

void GameWorldThread::BeginDetectCollisions()
{
	_detectCollisionStage.Begin();

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->_detectCollisionStage.Begin();
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

void GameWorldThread::JoinDetectCollisions()
{
	_detectCollisionStage.WaitForCompletion();

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->_detectCollisionStage.WaitForCompletion();
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
		_networkController->Start();

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
		_networkController->Start();

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

void GameWorldThread::SetPeerId(unsigned id)
{
	PhysicsWorkerThread::SetPeerId(id);

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->SetPeerId(id);
	}
}

void GameWorldThread::SetNumPeers(unsigned numPeers)
{
	PhysicsWorkerThread::SetNumPeers(numPeers);

	for (unsigned i = 0; i < _workers.size(); ++i)
	{
		_workers[i]->SetNumPeers(numPeers);
	}

	if (numPeers == 1)
	{
		for (int i = 0; i < _world->GetNumObjects(); ++i)
		{
			Physics::PhysicsObject* object = _world->GetObject(i);
			
			if (object->CanMigrate())
				object->SetOwnerId(0);
		}
	}
}