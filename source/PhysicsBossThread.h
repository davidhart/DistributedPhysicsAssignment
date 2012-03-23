// David Hart - 2012
// TODO: Document

#pragma once

#include "Threading.h"

struct WorldState;

class PhysicsBossThread : public Threading::Thread
{

public:

	unsigned ThreadMain();

	Threading::Event _physicsBegin;
	Threading::Event _physicsDone;

	WorldState* volatile _readState;
	WorldState* volatile _writeState;
	volatile double _delta;

};