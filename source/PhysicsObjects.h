// David Hart - 2012

#pragma once

#include "Vector.h"
#include "Color.h"
#include <vector>
#include <algorithm>

class World;
class IShapeCreator;

namespace Physics
{
	class PhysicsObject;

	enum eObjectType
	{
		OBJECT_BOX = 1,
		OBJECT_TRIANGLE = 2,
		OBJECT_BLOBBY = 3,
		OBJECT_BLOBBY_PART = 4,
	};

	struct Contact
	{
		Vector2d _contactNormal;
		double _penetrationDistance;
		double _totalMass;
		bool _static;

		Vector2d _velocityA;
		Vector2d _velocityB;

		void Reverse();
	};

	struct State
	{
		Vector2d _position;
		Vector2d _velocity;
	};

	struct Derivative
	{
		Vector2d _velocity;
		Vector2d _acceleration;
	};

	class Constraint
	{

	public:

		virtual Vector2d CalculateAcceleration(const State& state) const = 0;

	};

	class FixedEndSpringConstraint : public Constraint
	{

	public:

		FixedEndSpringConstraint();

		Vector2d CalculateAcceleration(const State& state) const;
		void SetFixedEndpoint(const Vector2d& position);
		void SetSpringConstant(double k);
		void SetDampingConstant(double b);
		void SetObjectSpaceAttachmentPoint(const Vector2d& position);

	private:
		
		Vector2d _endPoint;
		Vector2d _attachmentPoint;
		double _k, _b;

	};

	class LengthSpring : public Constraint
	{
	public:

		LengthSpring();

		Vector2d CalculateAcceleration(const State& state) const;

		void SetEndpoint(Physics::PhysicsObject* object);

		void SetSpringConstant(double k);
		void SetDampingConstant(double b);
		void SetLength(double length);

	private:

		Physics::PhysicsObject* _object;
		double _k, _b, _l;
	};

	class PhysicsObject
	{

	private:



	public:

		PhysicsObject();

		virtual void SetPosition(const Vector2d& position);
		Vector2d GetPosition() const;

		virtual void SetVelocity(const Vector2d& velocity);
		Vector2d GetVelocity() const;

		double GetMass() const;
		void SetMass(double mass);

		virtual void Integrate(double deltaTime);

		virtual void UpdateShape(World& world) = 0;

		virtual void ProcessCollisions(World& world) = 0;

		// TODO: double dispatch object types
		virtual bool TestCollision(PhysicsObject&, Contact&) { return false; }

		void AddContact(const Contact& contact);

		void AddConstraint(const Constraint* constraint);
		void RemoveConstraint(const Constraint* constraint);
		
		void SolveContacts(World& world);

		void SetColor(const Color& color);
		Color GetColor();

		virtual unsigned GetSerializationType() = 0;

		virtual void SetOwnerId(unsigned id);
		unsigned GetOwnerId();

		bool CanMigrate();

		void SetParent(PhysicsObject* parent);
		PhysicsObject* GetParent();
		
		void SetId(int id);
		int GetId();

	protected:
		
		State _state;

	private:

		virtual Vector2d CalculateAcceleration(const State& state) const;

		virtual Derivative EvaluateDerivative(const State& initialState, Derivative& derivative, double deltaTime);

		double _mass;

		static const int MAX_CONTACTS = 25;
		std::vector<Contact> _contacts;
		std::vector<const Constraint*> _constraints;

		Color _color;

		unsigned _ownerId;

		PhysicsObject* _parent;
		int _id;
	};

	class BoxObject : public PhysicsObject
	{

	public:

		BoxObject(int quad);

		void UpdateShape(World& world);
		void ProcessCollisions(World& world);
		bool TestCollision(PhysicsObject& object, Contact& contact);

		unsigned int GetSerializationType();

	private:

		int _quad;
	};


	class TriangleObject : public PhysicsObject
	{

	public:

		TriangleObject(int triangle);

		void UpdateShape(World& world);
		void ProcessCollisions(World& world);

		unsigned int GetSerializationType();

	private:

		int _triangle;
	};

	class BlobbyPart : public PhysicsObject
	{
	public:
		BlobbyPart();
		void UpdateShape(World& world);
		unsigned GetSerializationType();
		void ProcessCollisions(World& world);
	};

	class BlobbyObject : public PhysicsObject
	{

	public:

		BlobbyObject(World& world);
		
		void UpdateShape(World& world);
		unsigned GetSerializationType();
		void ProcessCollisions(World& world);

		static const int NUM_PARTS = 10;

		void SetPosition(const Vector2d& position);

		static int GetPart(int i);

		void SetOwnerId(unsigned id);

	private:

		BlobbyPart* _parts[NUM_PARTS];

		LengthSpring _partToParts[NUM_PARTS * (NUM_PARTS - 1)];

		LengthSpring _partToMid[NUM_PARTS];
		LengthSpring _midToPart[NUM_PARTS];

		int _triangles[NUM_PARTS];


		double _radius;

	};
}