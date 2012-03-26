// David Hart - 2012
// TODO: Document

#pragma once

#include "Threading.h"
#include <vector>

struct WorldState;

class PhysicsWorkerThread : public Threading::Thread
{

public:

	PhysicsWorkerThread();

	virtual void SetReadState(WorldState* worldState);
	virtual void SetWriteState(WorldState* worldState);

	void SetThreadId(unsigned id);
	void SetNumThreads(unsigned numThreads);

	unsigned GetStartIndex(unsigned count);
	unsigned GetEndIndex(unsigned count);

	virtual void BeginStep(double delta);
	virtual void WaitForStepCompletion();

	virtual void StopPhysics();

protected:

	virtual void PhysicsStep();

	WorldState* volatile _readState; // currently reading from
	WorldState* volatile _writeState; // currently writing to

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

	void SetReadState(WorldState* worldState);
	void SetWriteState(WorldState* worldState);
	void SetStepDelta(double delta);

	void BeginThreads();

	WorldState* SwapDrawState(WorldState* oldDrawState);

	unsigned TicksPerSec();
	void ResetTicksCounter();	

	void StopPhysics();

private:

	void BeginStep(double delta);
	
	void WaitForStepCompletion();
	void PhysicsStep();

	std::vector<PhysicsWorkerThread*> _workers;

	Threading::Mutex _stateSwapMutex;

	WorldState* volatile _freeState; // next buffer to write to

	unsigned _tickCount;

	// TODO: move these into timer class
	LARGE_INTEGER freq;
	LARGE_INTEGER startTime;
};
