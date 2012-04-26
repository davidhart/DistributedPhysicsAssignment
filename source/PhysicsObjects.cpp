// David Hart - 2012

#include "PhysicsObjects.h"
#include "World.h"
#include "ShapeBatch.h"
#include <algorithm>

using namespace Physics;

void Contact::Reverse()
{
	_contactNormal = -_contactNormal;
	_relativeVelocity = -_relativeVelocity;

	std::swap(_objectA, _objectB);
}

FixedEndSpringConstraint::FixedEndSpringConstraint() :
	_k(4),
	_b(0.5)
{
}

Vector2d FixedEndSpringConstraint::CalculateAcceleration(const State& state) const
{
	// TODO: Refactor into objectspace - worldspace function
	Vector2d worldSpaceAttachmentPoint = state._position + _attachmentPoint;

	Vector2d extension = worldSpaceAttachmentPoint - _endPoint;

	return -_k * extension - _b * state._velocity;
}

void FixedEndSpringConstraint::SetFixedEndpoint(const Vector2d& position)
{
	_endPoint = position;
}

void FixedEndSpringConstraint::SetSpringConstant(double k)
{
	_k = k;
}

void FixedEndSpringConstraint::SetDampingConstant(double b)
{
	_b = b;
}

void FixedEndSpringConstraint::SetObjectSpaceAttachmentPoint(const Vector2d& position)
{
	_attachmentPoint = position;
}

PhysicsObject::PhysicsObject() :
	_mass(1)
{
	_contacts.reserve(MAX_CONTACTS);
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

void PhysicsObject::SetColor(const Color& color)
{
	_color = color;
}

Color PhysicsObject::GetColor()
{
	return _color;
}

void PhysicsObject::AddContact(const Contact& contact)
{
	_contacts.push_back(contact);
	assert(_contacts.size() <= MAX_CONTACTS);
}

void PhysicsObject::AddConstraint(const Constraint* constraint)
{
	// Constraint should not already be in the list
	assert(std::find(_constraints.begin(), _constraints.end(), constraint) == _constraints.end());

	_constraints.push_back(constraint);	
}

void PhysicsObject::RemoveConstraint(const Constraint* constraint)
{
	std::vector<const Constraint*>::iterator it = std::find(_constraints.begin(), _constraints.end(), constraint);

	if (it != _constraints.end())
		_constraints.erase(it);

	// There should be no more copies of the constraint in the list
	assert(std::find(_constraints.begin(), _constraints.end(), constraint) == _constraints.end());
}

void PhysicsObject::SolveContacts()
{
	double elasticity = 0.8;
	double friction = 0.05;

	Vector2d position = _state._position;

	for (unsigned i = 0; i < _contacts.size(); ++i)
	{
		const Contact& contact = _contacts[i];

		// Separate the objects
		_state._position += contact._contactNormal * contact._penetrationDistance;

		// Apply friction
		if (abs(_state._velocity.dot(contact._contactNormal)) > Util::EPSILON)
		{
			Vector2d tangent = contact._contactNormal.tangent();
			double VdotT = tangent.dot(contact._relativeVelocity) / GetMass();
			_state._velocity -= tangent * VdotT * friction;
		}

		// For collisions against fixed objects reflect the velocity
		if (contact._static)
		{
			_state._velocity -= (1.0 + elasticity) * contact._contactNormal * contact._contactNormal.dot(_state._velocity);
		}
		// For collisions between moving objects preserve the momentum along the collision normal
		else
		{
			double normalImpulse = -((1.0 + elasticity) * contact._contactNormal.dot(contact._relativeVelocity)) / contact._totalMass;
			_state._velocity += contact._contactNormal * normalImpulse / GetMass();
		}
	}

	_contacts.clear();
}

Vector2d PhysicsObject::CalculateAcceleration(const State& state) const
{
	Vector2d acceleration(0);

	for (unsigned i = 0; i < _constraints.size(); ++i)
	{
		acceleration += _constraints[i]->CalculateAcceleration(state);
	}
	
	return acceleration + Vector2d(0, -9.81) /*- state._velocity*0.999*/; // Gravity and drag
}

void PhysicsObject::Integrate(double deltaTime)
{
	SolveContacts();

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

Derivative PhysicsObject::EvaluateDerivative(const State& initialState, Derivative& derivative, double deltaTime)
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
	SetColor(Color((float)Util::RandRange(0, 1), (float)Util::RandRange(0, 1), (float)Util::RandRange(0, 1), 1.0f));
}

void BoxObject::UpdateShape(World& world)
{
	Quad quad;
	quad._position = Vector2f(GetPosition());
	quad._rotation = 0;

	if (test != 1)
		quad._color = GetColor();
	else
		quad._color = Color(1.0f, 1.0f, 1.0f, 1.0f);

	world.UpdateQuad(_quad, quad);
}

void BoxObject::ProcessCollisions()
{
	Contact contact;
	contact._relativeVelocity = GetVelocity();
	contact._totalMass = 1 / GetMass();
	contact._static = true;
	Vector2d position = GetPosition();

	// top
	if (position.y() > 20 - 0.5)
	{
		contact._penetrationDistance = position.y() - 20 + 0.5;
		contact._contactNormal = Vector2d(0, -1);
		AddContact(contact);
	}

	// bottom
	if (position.y() < 0 + 0.5)
	{
		contact._penetrationDistance = -position.y() + 0.5;
		contact._contactNormal = Vector2d(0, 1);
		AddContact(contact);
	}
	
	// left
	if (position.x() > 20 - 0.5)
	{
		contact._penetrationDistance = position.x() - 20 + 0.5;
		contact._contactNormal = Vector2d(-1, 0);
		AddContact(contact);
	}

	// right
	if (position.x() < -20 + 0.5)
	{
		contact._penetrationDistance = -position.x() + -20 + 0.5;
		contact._contactNormal = Vector2d(1, 0);
		AddContact(contact);
	}
}

bool BoxObject::TestCollision(PhysicsObject& object, Contact& collision) // assume it is a box for now
{
	Vector2d dist = GetPosition() - object.GetPosition();

	if (dist.length() == 0)
		return false;
	

	if (abs(dist.x()) < 1 && abs(dist.y()) < 1)
	{

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

		dist = dist.normalize();

		Vector2d relVel = GetVelocity() - object.GetVelocity();
		
		if (relVel.dot(dist) < 0)
		{
			collision._totalMass = GetMass() + object.GetMass();
			collision._relativeVelocity = relVel;
			collision._static = false;
			collision._contactNormal = dist;
			return true;
		}
	}

	return false;
}

unsigned int BoxObject::GetSerializationType()
{
	return 0; // TODO: enum
}

TriangleObject::TriangleObject(int quad) :
	_triangle(quad)
{
}

void TriangleObject::UpdateShape(World&)
{

}
	
void TriangleObject::ProcessCollisions()
{
}

unsigned int TriangleObject::GetSerializationType()
{
	return 1; // TODO: enum
}