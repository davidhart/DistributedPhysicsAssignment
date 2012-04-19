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

	/*
	struct Collision
	{
		Vector2d _collisionNormal;
		double _penetrationDistance;

		Vector2d _objectAMomentum;
		Vector2d _objectBMomentum;

		enum eObject
		{
			OBJECT_A,
			OBJECT_B,
		};
	};*/

	struct Contact
	{
		Vector2d _contactNormal;
		double _penetrationDistance;
		Vector2d _relativeVelocity;
		//void* _object;
	};

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

		void SetColor(const Color& color);
		Color GetColor();

	protected:

		int test;

	private:

		Vector2d CalculateAcceleration(const State& state) const;

		Derivative EvaluateDerivative(const State& initialState, Derivative& derivative, double deltaTime);

		void SolveConstraints();

		Vector2d _jolt;
		State _state;
		double _mass;

		Vector2d _positionConstraint;

		static const int MAX_CONTACTS = 25;
		std::vector<Contact> _contacts;

		Color _color;
	};

	class BoxObject : public PhysicsObject
	{

	public:

		BoxObject(int quad);

		void UpdateShape(World& world);
		void ProcessCollisions();
		bool TestCollision(PhysicsObject& object, Contact& contact);

	private:

		int _quad;
	};


	class TriangleObject : public PhysicsObject
	{

	public:

		TriangleObject(int triangle);

		void UpdateShape(World& world);
		void ProcessCollisions();

	private:

		int _triangle;
	};

}