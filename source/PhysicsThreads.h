// David Hart - 2012
// TODO: Document

#pragma once

#include "Threading.h"
#include <vector>

class WorldState;

class PhysicsWorkerThread : public Threading::Thread
{

public:

	PhysicsWorkerThread();

	virtual void SetWorldState(WorldState* worldState);
	void SetThreadId(unsigned id);
	void SetNumThreads(unsigned numThreads);

	int GetStartIndex(unsigned count);
	int GetEndIndex(unsigned count);

	virtual void BeginStep(double delta);
	virtual void WaitForStepCompletion();

	virtual void StopPhysics();

protected:

	virtual void PhysicsStep();
	WorldState* _worldState;

private:

	unsigned ThreadMain();

	unsigned _threadId;
	unsigned _numThreads;

	volatile double _delta;
	volatile bool _haltPhysics;

	Threading::Event _physicsBegin;
	Threading::Event _physicsDone;
};

class PhysicsBossThread : public PhysicsWorkerThread
{

public:

	PhysicsBossThread();
	~PhysicsBossThread();

	void SetWorldState(WorldState* worldState);
	void SetStepDelta(double delta);

	void BeginThreads();

	unsigned TicksPerSec();
	void ResetTicksCounter();	

	void StopPhysics();

private:

	void BeginStep(double delta);
	
	void WaitForStepCompletion();
	void PhysicsStep();

	std::vector<PhysicsWorkerThread*> _workers;

	unsigned _tickCount;

	// TODO: move these into timer class
	LARGE_INTEGER freq;
	LARGE_INTEGER startTime;
};
