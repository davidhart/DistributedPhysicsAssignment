// David Hart - 2012

#include "PhysicsObjects.h"
#include "WorldState.h"
#include "ShapeBatch.h"

using namespace Physics;

void PhysicsObject::SetPosition(const Vector2d& position)
{
	_state._position = position;
}

Vector2d PhysicsObject::GetPosition() const
{
	return _state._position;
}

void PhysicsObject::SetVelocity(const Vector2d& velocity)
{
	_state._velocity = velocity;
}

Vector2d PhysicsObject::GetVelocity() const
{
	return _state._velocity;
}

Vector2d PhysicsObject::CalculateAcceleration(const State& state) const
{
	//return /*Vector2d(0, -9.81) + */-state._velocity*0.8;
	double k = 4;
	double b = 0;
	return - k * state._position - b * state._velocity;
}

void PhysicsObject::Integrate(double deltaTime)
{
	// Integrate using RK4 method
	Derivative a = EvaluateDerivative(_state, Derivative(), 0);
    Derivative b = EvaluateDerivative(_state, a, deltaTime*0.5);
    Derivative c = EvaluateDerivative(_state, b, deltaTime*0.5);
    Derivative d = EvaluateDerivative(_state, c, deltaTime);

	Derivative derivative;
	derivative._velocity = 1.0/6.0 * (a._velocity + 2.0*(b._velocity + c._velocity) + d._velocity);
	derivative._acceleration = 1.0/6.0 * (a._acceleration + 2.0*(b._acceleration + c._acceleration) + d._acceleration);

	SetPosition(_state._position += derivative._velocity * deltaTime);
	SetVelocity(_state._velocity += derivative._acceleration * deltaTime);
}

PhysicsObject::Derivative PhysicsObject::EvaluateDerivative(const State& initialState, Derivative& derivative, double deltaTime)
{
	State state;
	state._position = initialState._position + derivative._velocity * deltaTime;
	state._velocity = initialState._velocity + derivative._acceleration * deltaTime;

	Derivative d;
	d._velocity = state._velocity;
	d._acceleration = CalculateAcceleration(state);

	return d;
}

BoxObject::BoxObject(int quad) :
	_quad(quad)
{
}

void BoxObject::UpdateShape(WorldState& worldState)
{
	Quad quad;
	quad._position = Vector2f((float)GetPosition().x(), (float)GetPosition().y());
	quad._rotation = 0;
	quad._color = Color(1.0f, 1.0f, 1.0f, 1.0f);

	worldState.UpdateQuad(_quad, quad);
}

TriangleObject::TriangleObject(int quad) :
	_triangle(quad)
{
}

void TriangleObject::UpdateShape(WorldState& worldState)
{

}