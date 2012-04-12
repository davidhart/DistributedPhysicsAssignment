// David Hart - 2012

#include "PhysicsObjects.h"
#include "World.h"
#include "ShapeBatch.h"

using namespace Physics;

PhysicsObject::PhysicsObject() :
	_collisionNormal(0),
	_collisionMomentum(0),
	_mass(1)
{
}

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

double PhysicsObject::GetMass() const
{
	return _mass;
}

void PhysicsObject::SetMass(double mass)
{
	_mass = mass;
}

void PhysicsObject::AddCollisionConstraint(const Collision& collision, Collision::eObject object)
{
	Vector2d normal;

	if (Collision::OBJECT_A == object)
	{
		_collisionNormal += collision._collisionNormal;
		_collisionMomentum += collision._objectBMomentum;
	}
	else 
	{
		_collisionNormal -= collision._collisionNormal;
		_collisionMomentum += collision._objectAMomentum;
	}

	test = 1;
	_positionConstraint += normal * collision._penetrationDistance;

	//if (_state._velocity.dot(normal) > 0)
	//	return;

	/*
	if (_state._velocity.length() < 0.0001)
		_state._velocity = Vector2d(0);*/

	/*
	*/
}

void PhysicsObject::SolveConstraints()
{
	Vector2d momentum = GetVelocity() / GetMass() - _collisionMomentum;
	Vector2d normal = _collisionNormal.normalize();
	Vector2d velocityConstraint;

	if (normal.x() != 0 || normal.y() != 0)
	{
		double elasticity = 0.5;
		double friction = 0.2;
		// Friction
		Vector2d tangent = normal.tangent();
		double VdotT = tangent.dot(_state._velocity);
		_state._velocity -= tangent * VdotT * friction;

		// Elastic collision
		if (_state._velocity.dot(normal) < 0)
		{
			_state._velocity -= (2.0 - elasticity) * normal * normal.dot(_state._velocity);
			
			//if (_collisionMomentum.length() > 0.0001)
			//_state._velocity -= normal * normal.dot(momentum) / GetMass();
		}

		//if (momentum.length() < 0.0001)
		//	_state._velocity = Vector2d(0);		
	}

	_collisionMomentum = Vector2d(0);
	_collisionNormal = Vector2d(0);

	_state._position += _positionConstraint;
	_positionConstraint = Vector2d(0);
}

Vector2d PhysicsObject::CalculateAcceleration(const State& state) const
{
	
	return Vector2d(0, -9.81) /*+ -state._velocity*0.999*/; // Gravity and drag
	/*
	double k = 4;
	double b = 0;
	return - k * state._position - b * state._velocity;*/
}

void PhysicsObject::Integrate(double deltaTime)
{
	// TODO: solve constraints elsewhere
	SolveConstraints();

	test = 0;
	// Integrate using RK4 method
	Derivative d;
	Derivative a = EvaluateDerivative(_state, d, 0);
    Derivative b = EvaluateDerivative(_state, a, deltaTime*0.5);
    Derivative c = EvaluateDerivative(_state, b, deltaTime*0.5);
    d = EvaluateDerivative(_state, c, deltaTime);

	Derivative derivative;
	derivative._velocity = 1.0/6.0 * (a._velocity + 2.0*(b._velocity + c._velocity) + d._velocity);
	derivative._acceleration = 1.0/6.0 * (a._acceleration + 2.0*(b._acceleration + c._acceleration) + d._acceleration);

	SetPosition(_state._position += derivative._velocity * deltaTime);
	SetVelocity(_state._velocity += derivative._acceleration * deltaTime);

	ProcessCollisions();
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

	if (test != 1)
		quad._color = _color;
	else
		quad._color = Color(1.0f, 1.0f, 1.0f, 1.0f);

	world.UpdateQuad(_quad, quad);
}

void BoxObject::ProcessCollisions()
{
	Collision collision;
	collision._objectBMomentum = Vector2d(0);
	Vector2d position = GetPosition();

	// top
	if (position.y() > 20 - 0.5)
	{
		collision._penetrationDistance = position.y() - 20 + 0.5;
		collision._collisionNormal = Vector2d(0, -1);
		AddCollisionConstraint(collision, Collision::OBJECT_A);
	}

	// bottom
	if (position.y() < 0 + 0.5)
	{
		collision._penetrationDistance = -position.y() + 0.5;
		collision._collisionNormal = Vector2d(0, 1);
		AddCollisionConstraint(collision, Collision::OBJECT_A);
	}
	
	// left
	if (position.x() > 20 - 0.5)
	{
		collision._penetrationDistance = position.x() - 20 + 0.5;
		collision._collisionNormal = Vector2d(-1, 0);
		AddCollisionConstraint(collision, Collision::OBJECT_A);
	}

	// right
	if (position.x() < -20 + 0.5)
	{
		collision._penetrationDistance = -position.x() + -20 + 0.5;
		collision._collisionNormal = Vector2d(1, 0);
		AddCollisionConstraint(collision, Collision::OBJECT_A);
	}
}

bool BoxObject::TestCollision(PhysicsObject& object, Collision& collision) // assume it is a box for now
{

	Vector2d dist = GetPosition() - object.GetPosition();

	if (dist.length() == 0)
		return false;

	if (abs(dist.x()) < 1 && abs(dist.y()) < 1)
	{
		collision._objectAMomentum =  GetVelocity() / GetMass();
		collision._objectBMomentum = object.GetVelocity() / object.GetMass();

		if (abs(dist.x()) < abs(dist.y()))
		{
			dist.x(0);
			collision._penetrationDistance = (1 - abs(dist.y())) / 2;
		}
		else
		{
			dist.y(0);
			collision._penetrationDistance = (1 - abs(dist.x())) / 2;
		}
		
		collision._collisionNormal = dist.normalize();
		return true;
	}

	return false;
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