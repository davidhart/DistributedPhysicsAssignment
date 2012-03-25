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
	virtual void SetStepDelta(double delta);

	virtual void BeginStep();
	virtual void WaitForStepCompletion();

	void SetThreadId(unsigned id);
	void SetNumThreads(unsigned numThreads);


	unsigned GetStartIndex(unsigned count);
	unsigned GetEndIndex(unsigned count);

private:

	unsigned ThreadMain();

	unsigned _threadId;
	unsigned _numThreads;
	WorldState* volatile _readState;
	WorldState* volatile _writeState;
	volatile double _delta;

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
	void BeginStep();
	void WaitForStepCompletion();

private:

	std::vector<PhysicsWorkerThread*> _workers;

};
