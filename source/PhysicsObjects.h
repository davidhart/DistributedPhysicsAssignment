// David Hart - 2012

#pragma once

#include "Vector.h"
#include "Color.h"
#include <vector>
#include <algorithm>

class World;

namespace Physics
{
	class PhysicsObject;

	struct Contact
	{
		Vector2d _contactNormal;
		double _penetrationDistance;
		Vector2d _relativeVelocity;
		double _totalMass;
		bool _static;

		void* _objectA;
		void* _objectB;

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


	class PhysicsObject
	{

	private:



	public:

		PhysicsObject();

		void SetPosition(const Vector2d& position);
		virtual Vector2d GetPosition() const;

		void SetVelocity(const Vector2d& velocity);
		Vector2d GetVelocity() const;

		double GetMass() const;
		void SetMass(double mass);

		void Integrate(double deltaTime);

		virtual void UpdateShape(World& world) = 0;

		virtual void ProcessCollisions() = 0;

		// TODO: double dispatch object types
		virtual bool TestCollision(PhysicsObject&, Contact&) { return false; }

		void AddContact(const Contact& contact);

		void AddConstraint(const Constraint* constraint);
		void RemoveConstraint(const Constraint* constraint);

		void SetColor(const Color& color);
		Color GetColor();

		virtual unsigned int GetSerializationType() = 0;

	protected:

		int test;

	private:

		Vector2d CalculateAcceleration(const State& state) const;

		Derivative EvaluateDerivative(const State& initialState, Derivative& derivative, double deltaTime);

		void SolveContacts();

		Vector2d _jolt;
		State _state;
		double _mass;

		Vector2d _positionConstraint;

		static const int MAX_CONTACTS = 25;
		std::vector<Contact> _contacts;
		std::vector<const Constraint*> _constraints;

		Color _color;
	};

	class BoxObject : public PhysicsObject
	{

	public:

		BoxObject(int quad);

		void UpdateShape(World& world);
		void ProcessCollisions();
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
		void ProcessCollisions();

		unsigned int GetSerializationType();

	private:

		int _triangle;
	};

}