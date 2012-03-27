// David Hart - 2012

#pragma once

#include "Vector.h"

class WorldState;

namespace Physics
{

class PhysicsObject
{

private:

	struct Derivative
	{
		Vector2d _velocity;
		Vector2d _acceleration;
	};

	struct State
	{
		Vector2d _position;
		Vector2d _velocity;
	};

public:

	void SetPosition(const Vector2d& position);
	virtual Vector2d GetPosition() const;

	void SetVelocity(const Vector2d& velocity);
	Vector2d GetVelocity() const;

	void Integrate(double deltaTime);

	Vector2d CalculateAcceleration(const State& state) const;

	virtual void UpdateShape(WorldState& worldState) = 0;

private:

	Derivative EvaluateDerivative(const State& initialState, Derivative& derivative, double deltaTime);

	State _state;
	double _mass;
	
};


class BoxObject : public PhysicsObject
{

public:
	
	BoxObject(int quad);
	
	void UpdateShape(WorldState& worldState);

private:

	int _quad;

};


class TriangleObject : public PhysicsObject
{

public:

	TriangleObject(int triangle);

	void UpdateShape(WorldState& worldState);

private:

	int _triangle;
};

}