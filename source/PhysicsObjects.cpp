// David Hart - 2012

#include "PhysicsObjects.h"
#include "World.h"
#include "ShapeBatch.h"
#include <algorithm>

using namespace Physics;

void Contact::Reverse()
{
	_contactNormal = -_contactNormal;
	std::swap(_velocityA, _velocityB);
}

FixedEndSpringConstraint::FixedEndSpringConstraint() :
	_k(4),
	_b(2)
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

LengthSpring::LengthSpring() :
	_k(300),
	_b(4),
	_l(1)
{
}

Vector2d LengthSpring::CalculateAcceleration(const State& state) const
{
	Vector2d direction = _object->GetPosition() - state._position;
	double length = direction.length();

	direction /= length;

	double stretch = length - _l;

	return (_k * direction * stretch - _b * direction * direction.dot(state._velocity)) / 2.0;
}

void LengthSpring::SetEndpoint(Physics::PhysicsObject* object)
{
	_object = object;
}

void LengthSpring::SetLength(double length)
{
	_l = length;
}

void LengthSpring::SetSpringConstant(double k)
{
	_k = k;
}

void LengthSpring::SetDampingConstant(double b)
{
	_b = b;
}

PhysicsObject::PhysicsObject() :
	_mass(1),
	_ownerId(0)
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

unsigned int TriangleObject::GetSerializationType()
{
	return 1; // TODO: enum
}

void PhysicsObject::SetOwnerId(unsigned id)
{
	_ownerId = id;
}

unsigned PhysicsObject::GetOwnerId()
{
	return _ownerId;
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

void PhysicsObject::SolveContacts(World& world)
{
	ProcessCollisions(world);

	// Sort contacts by their normal projected onto the gravity vector
	// This prevents collisions of objects on top causing lower objects to sink
	// into each other and the ground, this implementation works for gravity down
	// the y axis but a more general solution based on the forces acting on an object
	// would be preferred
	for (unsigned i = 0; i < _contacts.size(); ++i)
	{
		for (unsigned j = 0; j < _contacts.size() - i - 1; ++j)
		{
			if (_contacts[j+1]._contactNormal.y() < _contacts[j]._contactNormal.y())
			{
				std::swap(_contacts[j+1], _contacts[j]);
			}
		}
	}


	double elasticity = 0.8;
	double friction = 0.05;

	Vector2d originalPosition = _state._position;

	for (unsigned i = 0; i < _contacts.size(); ++i)
	{
		const Contact& contact = _contacts[i];

		Vector2d relVel = _state._velocity - contact._velocityB;

		// Apply friction
		if (abs(_state._velocity.dot(contact._contactNormal)) > Util::EPSILON)
		{
			Vector2d tangent = contact._contactNormal.tangent();
			double VdotT = tangent.dot(relVel) / GetMass();
			_state._velocity -= tangent * VdotT * friction;
		}
		
		// Conservation of momentum
		double relVeldotN = relVel.dot(contact._contactNormal);
		if (relVeldotN < 0)
		{
			double normalImpulse = -((1.0 + elasticity) * contact._contactNormal.dot(relVel)) / contact._totalMass;
			_state._velocity += contact._contactNormal * normalImpulse / GetMass();
		}

		// Separate the objects
		/*
		double deltaDotNormal = (_state._position - originalPosition).dot(contact._contactNormal);
		*/

		// Stop objects above causing delta positions
		if (contact._contactNormal.y() > 0)
		{
			_state._position += contact._contactNormal * Util::Max(contact._penetrationDistance * 2 / 3.0, 0.0);
		}
		else
		{
			_state._position += contact._contactNormal * Util::Max(contact._penetrationDistance / 3.0, 0.0);
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
	_contacts.clear();

	// Integrate using RK4 method
	Derivative d;
	Derivative a = EvaluateDerivative(_state, d, 0);
    Derivative b = EvaluateDerivative(_state, a, deltaTime*0.5);
    Derivative c = EvaluateDerivative(_state, b, deltaTime*0.5);
    d = EvaluateDerivative(_state, c, deltaTime);

	Derivative derivative;
	derivative._velocity = 1.0/6.0 * (a._velocity + 2.0*(b._velocity + c._velocity) + d._velocity);
	derivative._acceleration = 1.0/6.0 * (a._acceleration + 2.0*(b._acceleration + c._acceleration) + d._acceleration);

	_state._position += derivative._velocity * deltaTime;
	_state._velocity += derivative._acceleration * deltaTime;
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

	quad._color = world.GetObjectColor(*this);

	world.UpdateQuad(_quad, quad);
}

void BoxObject::ProcessCollisions(World& world)
{
	Contact contact;
	//contact._relativeVelocity = GetVelocity();
	contact._velocityA = GetVelocity();
	contact._velocityB = Vector2d(0);
	contact._totalMass = 1 / GetMass();
	contact._static = true;
	Vector2d position = GetPosition();

	// top
	if (position.y() > world.GetWorldMax().y() - 0.5)
	{
		contact._penetrationDistance = position.y() -  world.GetWorldMax().y() + 0.5;
		contact._contactNormal = Vector2d(0, -1);
		AddContact(contact);
	}

	// bottom
	if (position.y() < world.GetWorldMin().y() + 0.5)
	{
		contact._penetrationDistance = world.GetWorldMin().y() - position.y() + 0.5;
		contact._contactNormal = Vector2d(0, 1);
		AddContact(contact);
	}
	
	// left
	if (position.x() > world.GetWorldMax().x() - 0.5)
	{
		contact._penetrationDistance = position.x() - world.GetWorldMax().x() + 0.5;
		contact._contactNormal = Vector2d(-1, 0);
		AddContact(contact);
	}

	// right
	if (position.x() < world.GetWorldMin().x() + 0.5)
	{
		contact._penetrationDistance = world.GetWorldMin().x() - position.x() + 0.5;
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

		/*
		Vector2d relVel = GetVelocity() - object.GetVelocity();
		
		if (relVel.dot(dist) < 0)
		{*/
			collision._totalMass = GetMass() + object.GetMass();
			//collision._relativeVelocity = relVel;
			collision._static = false;
			collision._contactNormal = dist;
			collision._velocityA = GetVelocity();
			collision._velocityB = object.GetVelocity();
			return true;
		/*}*/
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
	
void TriangleObject::ProcessCollisions(World&)
{
}

void BlobbyPart::UpdateShape(World&)
{
}

unsigned BlobbyPart::GetSerializationType()
{
	return 4;
}

void BlobbyPart::ProcessCollisions(World& world)
{
	Contact contact;
	contact._velocityA = GetVelocity();
	contact._velocityB = Vector2d(0);
	contact._totalMass = 1 / GetMass();
	contact._static = true;
	Vector2d position = GetPosition();

	// top
	if (position.y() > world.GetWorldMax().y())
	{
		contact._penetrationDistance = position.y() -  world.GetWorldMax().y();
		contact._contactNormal = Vector2d(0, -1);
		AddContact(contact);
	}

	// bottom
	if (position.y() < world.GetWorldMin().y())
	{
		contact._penetrationDistance = world.GetWorldMin().y() - position.y();
		contact._contactNormal = Vector2d(0, 1);
		AddContact(contact);
	}
	
	// left
	if (position.x() > world.GetWorldMax().x())
	{
		contact._penetrationDistance = position.x() - world.GetWorldMax().x();
		contact._contactNormal = Vector2d(-1, 0);
		AddContact(contact);
	}

	// right
	if (position.x() < world.GetWorldMin().x())
	{
		contact._penetrationDistance = world.GetWorldMin().x() - position.x();
		contact._contactNormal = Vector2d(1, 0);
		AddContact(contact);
	}
}

int BlobbyObject::GetPart(int i)
{
	if (i < 0) i+= NUM_PARTS;

	return i % NUM_PARTS;
}

BlobbyObject::BlobbyObject(World& world) :
	_radius(1.0)
{
	double angle = 2.0 * PI / NUM_PARTS;

	for (int i = 0; i < NUM_PARTS; ++i)
	{
		_triangles[i] = world.CreateTriangle();

		_parts[i] = world.AddBlobbyPart();

		_parts[i]->SetPosition(_state._position + _radius * Vector2d(sin(i * angle), cos(i * angle)));
	}

	double adjLength = (_parts[1]->GetPosition() - _parts[0]->GetPosition()).length();
	double flexLength = (_parts[2]->GetPosition() - _parts[0]->GetPosition()).length();

	int partId = 0;
	for (int i = 0; i < NUM_PARTS; ++i)
	{
		_parts[i]->AddConstraint(_partToMid + i);
		_partToMid[i].SetEndpoint(this);
		_partToMid[i].SetLength(_radius);
		_partToMid[i].SetSpringConstant(250);

		AddConstraint(_midToPart+i);
		_midToPart[i].SetLength(_radius);
		_midToPart[i].SetEndpoint(_parts[i]);
		_midToPart[i].SetSpringConstant(250);

		for (int j = 0; j < NUM_PARTS; ++j)
		{
			if (i != j)
			{
				_partToParts[partId].SetEndpoint(_parts[j]);
				double length = (_parts[j]->GetPosition() - _parts[i]->GetPosition()).length();
				_partToParts[partId].SetLength(length);
				_parts[i]->AddConstraint(_partToParts + partId);
				//_partToParts[partId].SetDampingConstant(2);
				partId++;
			}
		}
	}
		/*
		// Connect each point to the midpoint of the object

		// Connect each point to the opposite point
		_parts[i]->AddConstraint(_partToOpposite + i);
		_partToOpposite[i].SetEndpoint(_parts[GetPart(i + NUM_PARTS / 2)]);
		_partToOpposite[i].SetLength(_radius * 2);

		// Connect each point to the next and previous points
		_parts[i]->AddConstraint(_partToNext + i);
		_partToNext[i].SetEndpoint(_parts[GetPart(i+1)]);
		_partToNext[i].SetLength(adjLength);

		_parts[i]->AddConstraint(_partToPrev + i);
		_partToPrev[i].SetEndpoint(_parts[GetPart(i-1)]);
		_partToPrev[i].SetLength(adjLength);

		// Connect ecah point to the second and previous next points
		_parts[i]->AddConstraint(_flexionNext + i);
		_flexionNext[i].SetEndpoint(_parts[GetPart(i+2)]);
		_flexionNext[i].SetLength(flexLength);

		_parts[i]->AddConstraint(_flexionPrev + i);
		_flexionPrev[i].SetEndpoint(_parts[GetPart(i-2)]);
		_flexionPrev[i].SetLength(flexLength);
		*/
}

void BlobbyObject::UpdateShape(World& world)
{
	Color c = world.GetObjectColor(*this);
	Triangle t;
	t._color = c;
	t._points[0] = Vector2f(GetPosition());
	for (int i = 0; i < NUM_PARTS - 1; ++i)
	{
		t._points[2] = Vector2f(_parts[i]->GetPosition());
		t._points[1] = Vector2f(_parts[i + 1]->GetPosition());

		world.UpdateTriangle(_triangles[i], t);
	}

	t._points[2] = Vector2f(_parts[NUM_PARTS - 1]->GetPosition());
	t._points[1] = Vector2f(_parts[0]->GetPosition());
	world.UpdateTriangle(_triangles[NUM_PARTS - 1], t);
}

unsigned BlobbyObject::GetSerializationType()
{
	return 3;
}

void BlobbyObject::SetPosition(const Vector2d& position)
{
	// Move sub objects relative to main objects
	for (int i = 0; i < NUM_PARTS; ++i)
	{
		Vector2d delta = _parts[i]->GetPosition() - GetPosition();
		_parts[i]->SetPosition(position + delta);
	}

	PhysicsObject::SetPosition(position);
}