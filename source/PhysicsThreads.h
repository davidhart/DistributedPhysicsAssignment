// David Hart - 2012
// TODO: Document

#pragma once

#include "NetworkController.h"
#include "Threading.h"
#include <vector>

class World;

class PhysicsStage
{

public:

	PhysicsStage();
	void Begin();
	void Completed();
	void WaitForBegin();
	void WaitForCompletion();

private:

	Threading::Event _begin;
	Threading::Event _end;
};

class PhysicsWorkerThread : public Threading::Thread
{

public:

	friend class GameWorldThread;

	PhysicsWorkerThread();

	void SetThreadId(unsigned id);
	void SetNumThreads(unsigned numThreads);

	int GetStartIndex(unsigned count);
	int GetEndIndex(unsigned count);

protected:

	virtual void PhysicsStep();
	void Integrate();
	void BroadPhase();
	void SolveCollisions();
	World* _world;

private:

	unsigned ThreadMain();

	unsigned _threadId;
	unsigned _numThreads;

	volatile double _delta;
	volatile bool _haltPhysics;

	PhysicsStage _integrationStage;
	PhysicsStage _broadPhaseStage;
	PhysicsStage _solveCollisionStage;
};

class GameWorldThread : public PhysicsWorkerThread
{

	friend class SessionMasterController;
	friend class WorkerController;

public:

	GameWorldThread();
	~GameWorldThread();

	void SetWorld(World* worldState);
	void SetStepDelta(double delta);

	void BeginThreads();

	unsigned TicksPerSec();
	void ResetTicksCounter();	

	void StopPhysics();
	
	void SetMouseState(int x, int y, bool leftButton, bool rightButton);

	void CreateSession();
	void JoinSession();
	void TerminateSession();

private:

	void ExitWorkers();

	void BeginIntegration();
	void BeginBroadphase();
	void BeginSolveCollisions();

	void JoinIntegration();
	void JoinBroadphase();
	void JoinSolveCollisions();

	void PhysicsStep();

	void SanityCheckObjectsInBuckets();

	std::vector<PhysicsWorkerThread*> _workers;

	bool _shuttingDown;
	unsigned _tickCount;

	// TODO: move these into timer class
	LARGE_INTEGER freq;
	LARGE_INTEGER startTime;

	Threading::Mutex _stateChangeMutex;

	enum eState
	{
		STATE_STANDALONE,
		STATE_SESSIONMASTER,
		STATE_SESSIONWORKER,
	};

	eState _state;

	NetworkController* _networkController;
};
