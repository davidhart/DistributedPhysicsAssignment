// David Hart - 2012

#pragma once

#include "NetworkController.h"
#include "Threading.h"
#include "Timer.h"
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
	friend class ObjectExchange;

	PhysicsWorkerThread();

	void SetThreadId(unsigned id);
	void SetNumThreads(unsigned numThreads);

	int GetPeerStartIndex(unsigned count);
	int GetPeerEndIndex(unsigned count);

	void SetPeerId(unsigned id);
	virtual void SetNumPeers(unsigned numPeers);

	static int GetStartIndexForId(unsigned id, unsigned numIds, unsigned count);
	static int GetEndIndexForId(unsigned id, unsigned numIds, unsigned count);

protected:

	virtual void PhysicsStep();
	void Integrate();
	void BroadPhase();
	void SolveCollisions();
	void DetectCollisions();
	World* _world;

private:

	unsigned ThreadMain();

	unsigned _threadId;
	unsigned _numThreads;

	unsigned _peerId;
	unsigned _numPeers;

	volatile double _delta;
	volatile bool _haltPhysics;

	PhysicsStage _integrationStage;
	PhysicsStage _broadPhaseStage;
	PhysicsStage _detectCollisionStage;
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
	double GetStepDelta();

	void BeginThreads();

	unsigned TicksPerSec();
	void ResetTicksCounter();	

	void StopPhysics();
	
	void SetMouseState(int x, int y, bool leftButton, bool rightButton);

	void CreateSession();
	void JoinSession();
	void TerminateSession();

	void SetPeerId(unsigned id);
	void SetNumPeers(unsigned numPeers);

	void GetLastNetworkingMessage(std::string& out);

private:

	void ExitWorkers();

	void BeginIntegration();
	void BeginBroadphase();
	void BeginDetectCollisions();
	void BeginSolveCollisions();

	void JoinIntegration();
	void JoinBroadphase();
	void JoinDetectCollisions();
	void JoinSolveCollisions();

	void PhysicsStep();

	void SanityCheckObjectsInBuckets();

	std::vector<PhysicsWorkerThread*> _workers;

	bool _shuttingDown;
	unsigned _tickCount;

	Timer _timer;

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
