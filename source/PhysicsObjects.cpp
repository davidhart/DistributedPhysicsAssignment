// David Hart - 2012

#include "PhysicsObjects.h"
#include "World.h"
#include "ShapeBatch.h"
#include "AABB.h"
#include <algorithm>

using namespace Physics;

bool Contact::BoxBoxCollision(const PhysicsObject& a, const PhysicsObject& b)
{
	AABB abb(a.GetPosition() - Vector2d(0.5, 0.5), a.GetPosition() + Vector2d(0.5, 0.5));
	AABB bbb(b.GetPosition() - Vector2d(0.5, 0.5), b.GetPosition() + Vector2d(0.5, 0.5));

	if (abb.Intersects(bbb, _penetrationDistance, _contactNormal))
	{
		_static = false;
		_velocityA = a.GetVelocity();
		_massA = a.GetMass();
		_velocityB = b.GetVelocity();
		_massB = b.GetMass();

		return true;
	}

	return false;
}

bool Contact::BoxPointCollision(const PhysicsObject& a, const PhysicsObject& b)
{
	AABB abb(a.GetPosition() - Vector2d(0.5, 0.5), a.GetPosition() + Vector2d(0.5, 0.5));
	Vector2d point = b.GetPosition();

	if (abb.Intersects(point, _penetrationDistance, _contactNormal))
	{
		_static = false;
		_velocityA = a.GetVelocity();
		_massA = a.GetMass();
		_velocityB = b.GetVelocity();
		_massB = b.GetMass();

		return true;
	}

	return false;
}

bool Contact::PointBoxCollision(const PhysicsObject& a, const PhysicsObject& b)
{
	if (BoxPointCollision(b, a))
	{
		Reverse();
		return true;
	}

	return false;
}

void Contact::Reverse()
{
	_contactNormal = -_contactNormal;
	std::swap(_velocityA, _velocityB);
	std::swap(_massA, _massB);
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
	_k(80),
	_b(1.5),
	_l(1)
{
}

Vector2d LengthSpring::CalculateAcceleration(const State& state) const
{
	Vector2d direction = _object->GetPosition() - state._position;
	double length = direction.length();

	direction /= length;

	double stretch = length - _l;

	return (_k * direction * stretch - _b * direction * direction.dot(state._velocity - _object->GetVelocity())) / 2.0;
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
	_ownerId(0),
	_parent(NULL),
	_id(-1)
{
	_contacts.reserve(MAX_CONTACTS);

	SetColor(Color((float)Util::RandRange(0, 1), (float)Util::RandRange(0, 1), (float)Util::RandRange(0, 1), 1.0f));
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
	return OBJECT_TRIANGLE;
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
	//assert(_contacts.size() <= MAX_CONTACTS);
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

		Vector2d relVel = _state._velocity * contact._massA - contact._velocityB * contact._massB;

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
			double normalImpulse = -((1.0 + elasticity) * contact._contactNormal.dot(relVel)) * (contact._massA / (contact._massA + contact._massB));
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

bool PhysicsObject::CanMigrate()
{
	return _parent == NULL;
}

void PhysicsObject::SetParent(PhysicsObject* parent)
{
	_parent = parent;
}

PhysicsObject* PhysicsObject::GetParent()
{
	return _parent;
}

void PhysicsObject::SetId(int id)
{
	_id = id;
}

int PhysicsObject::GetId()
{
	return _id;
}
BoxObject::BoxObject(int quad) :
	_quad(quad)
{
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
	contact._massA = GetMass();
	contact._velocityB = Vector2d(0);
	contact._massB = 0;
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


unsigned int BoxObject::GetSerializationType()
{
	return OBJECT_BOX;
}

bool BoxObject::TestCollision(PhysicsObject& object, Contact& contact) // assume it is a box for now
{
	return object.TestCollision(*this, contact);
}

bool BoxObject::TestCollision(BoxObject& object, Contact& contact)
{
	return contact.BoxBoxCollision(object, *this);
}

bool BoxObject::TestCollision(TriangleObject& object, Contact& contact)
{
	return contact.BoxBoxCollision(object, *this);
}

bool BoxObject::TestCollision(BlobbyPart& object, Contact& contact)
{
	return contact.PointBoxCollision(object, *this);
}

TriangleObject::TriangleObject(int quad) :
	_triangle(quad)
{
}

void TriangleObject::UpdateShape(World& world)
{
	Triangle t;
	t._points[0] = Vector2f((float)GetPosition().x() - 0.5f, (float)GetPosition().y() - 0.5f);
	t._points[1] = Vector2f((float)GetPosition().x() + 0.5f, (float)GetPosition().y() - 0.5f);
	t._points[2] = Vector2f((float)GetPosition().x(), (float)GetPosition().y() + 0.5f);
	t._color = world.GetObjectColor(*this);

	world.UpdateTriangle(_triangle, t);
}

void TriangleObject::ProcessCollisions(World& world)
{
	Contact contact;
	//contact._relativeVelocity = GetVelocity();
	contact._velocityA = GetVelocity();
	contact._massA = GetMass();
	contact._velocityB = Vector2d(0);
	contact._massB = 0;
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

bool TriangleObject::TestCollision(PhysicsObject& object, Contact& contact)
{
	return object.TestCollision(*this, contact);
}

bool TriangleObject::TestCollision(BoxObject& object, Contact& contact)
{
	return contact.BoxBoxCollision(object, *this);
}

bool TriangleObject::TestCollision(TriangleObject& object, Contact& contact)
{
	return contact.BoxBoxCollision(object, *this);
}

bool TriangleObject::TestCollision(BlobbyPart& object, Contact& contact)
{
	return contact.PointBoxCollision(object, *this);
}


BlobbyPart::BlobbyPart()
{
}

void BlobbyPart::UpdateShape(World&)
{
}

unsigned BlobbyPart::GetSerializationType()
{
	return OBJECT_BLOBBY_PART;
}

void BlobbyPart::ProcessCollisions(World& world)
{
	Contact contact;
	contact._velocityA = GetVelocity();
	contact._massA = GetMass();
	contact._velocityB = Vector2d(0);
	contact._massB = 0;
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

bool BlobbyPart::TestCollision(PhysicsObject& object, Contact& contact)
{
	return object.TestCollision(*this, contact);
}

bool BlobbyPart::TestCollision(BoxObject& object, Contact& contact)
{
	return contact.BoxPointCollision(object, *this);
}

bool BlobbyPart::TestCollision(TriangleObject& object, Contact& contact)
{
	return contact.BoxPointCollision(object, *this);
}

// Blobby parts can't collide with each other
bool BlobbyPart::TestCollision(BlobbyPart&, Contact&)
{
	return false;
}

int BlobbyObject::GetPart(int i)
{
	if (i < 0) i+= NUM_PARTS;

	return i % NUM_PARTS;
}

BlobbyObject::BlobbyObject(World& world) :
	_radius(2.0)
{
	double angle = 2.0 * PI / NUM_PARTS;

	for (int i = 0; i < NUM_PARTS; ++i)
	{
		_triangles[i] = world.CreateTriangle();

		_parts[i] = world.AddBlobbyPart();
		_parts[i]->SetParent(this);
		_parts[i]->SetMass(1.0);

		_parts[i]->SetPosition(_state._position + _radius * Vector2d(sin(i * angle), cos(i * angle)));
	}
	int partId = 0;
	for (int i = 0; i < NUM_PARTS; ++i)
	{
		_parts[i]->AddConstraint(_partToMid + i);
		_partToMid[i].SetEndpoint(this);
		_partToMid[i].SetLength(_radius);

		AddConstraint(_midToPart+i);
		_midToPart[i].SetLength(_radius);
		_midToPart[i].SetEndpoint(_parts[i]);
		//_midToPart[i].SetSpringConstant(250);

		const double maxLength = _radius * 2;
		const double maxStrength = 1000;
		const double minStrength = 200;

		for (int j = 0; j < NUM_PARTS; j++)
		{
			if (i != j)
			{
				_partToParts[partId].SetEndpoint(_parts[j]);
				double length = (_parts[j]->GetPosition() - _parts[i]->GetPosition()).length();

				double strength = (1.0 - length / maxLength) * (maxStrength - minStrength) + minStrength;
				_partToParts[partId].SetLength(length);
				_partToParts[partId].SetSpringConstant(strength);
				_parts[i]->AddConstraint(_partToParts + partId);
				//_partToParts[partId].SetDampingConstant(2);
				partId++;
			}
		}
	}
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
	return OBJECT_BLOBBY;
}

void BlobbyObject::ProcessCollisions(World&)
{

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

void BlobbyObject::SetOwnerId(unsigned id)
{
	for (int i = 0; i < NUM_PARTS; ++i)
	{
		_parts[i]->SetOwnerId(id);
	}

	PhysicsObject::SetOwnerId(id);
}

// The midpoint of the blobby object can't collide with anything
bool BlobbyObject::TestCollision(PhysicsObject&, Contact&)
{
	return false;
}

bool BlobbyObject::TestCollision(BoxObject&, Contact&)
{
	return false;
}

bool BlobbyObject::TestCollision(TriangleObject&, Contact&)
{
	return false;
}

bool BlobbyObject::TestCollision(BlobbyPart&, Contact&)
{
	return false;
}