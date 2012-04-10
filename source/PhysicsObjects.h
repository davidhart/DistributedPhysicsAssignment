// David Hart - 2012

#pragma once

#include "Vector.h"
#include "Color.h"

class World;

namespace Physics
{
	struct Collision
	{
		Vector2d _collisionNormal;
		// TODO: penetration distance?
		// TODO: contact point
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

		void SetPosition(const Vector2d& position);
		virtual Vector2d GetPosition() const;

		void SetVelocity(const Vector2d& velocity);
		Vector2d GetVelocity() const;

		void Integrate(double deltaTime);

		virtual void UpdateShape(World& world) = 0;

		virtual void ProcessCollisions() = 0;
		void CollisionResponse(const Vector2d& normal);

		// TODO: double dispatch object types
		virtual bool TestCollision(PhysicsObject& object, Collision& collision) { return false; };

		int test;

	private:

		Vector2d CalculateAcceleration(const State& state) const;

		Derivative EvaluateDerivative(const State& initialState, Derivative& derivative, double deltaTime);

		State _state;
		double _mass;

	};

	class BoxObject : public PhysicsObject
	{

	public:

		BoxObject(int quad);

		void UpdateShape(World& world);
		void ProcessCollisions();
		bool TestCollision(PhysicsObject& object, Collision& collision);

	private:

		int _quad;
		Color _color;
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