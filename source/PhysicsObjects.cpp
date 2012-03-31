// David Hart - 2012

#include "PhysicsObjects.h"
#include "World.h"
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
	
	return Vector2d(0, -9.81) + -state._velocity*0.999; // Gravity and friction
	/*
	double k = 4;
	double b = 0;
	return - k * state._position - b * state._velocity;*/
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

	ProcessCollisions();
}

void PhysicsObject::CollisionResponse(const Vector2d& normal)
{
	if (_state._velocity.length() < 0.0001)
		_state._velocity = Vector2d(0);

	double elasticity = 0.2;
	double friction = 0.4;
	// Elastic collision
	_state._velocity -= (2.0 - elasticity) * normal * normal.dot(_state._velocity);

	// Friction
	Vector2d tangent = normal.tangent();
	double VdotT = tangent.dot(_state._velocity);
	_state._velocity -= tangent * VdotT * friction;
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
	_quad(quad),
	_color((float)Util::RandRange(0, 1), (float)Util::RandRange(0, 1), (float)Util::RandRange(0, 1), 1.0f)
{
}

void BoxObject::UpdateShape(World& world)
{
	Quad quad;
	quad._position = Vector2f(GetPosition());
	quad._rotation = 0;
	quad._color = _color;

	world.UpdateQuad(_quad, quad);
}

void BoxObject::ProcessCollisions()
{
	Vector2d position = GetPosition();
	if (position.y() > 20 - 0.5)
	{
		position.y(20 - 0.5);
		SetPosition(position);
		CollisionResponse(Vector2d(0, -1));	
	}

	if (position.y() < 0 + 0.5)
	{
		position.y(0 + 0.5);
		SetPosition(position);
		CollisionResponse(Vector2d(0, 1));	
	}

	if (position.x() > 20 - 0.5)
	{
		position.x(20 - 0.5);
		SetPosition(position);
		CollisionResponse(Vector2d(-1, 0));	
	}

	if (position.x() < -20 + 0.5)
	{
		position.x(-20 + 0.5);
		SetPosition(position);
		CollisionResponse(Vector2d(1, 0));	
	}
}

TriangleObject::TriangleObject(int quad) :
	_triangle(quad)
{
}

void TriangleObject::UpdateShape(World& world)
{

}
	
void TriangleObject::ProcessCollisions()
{
}