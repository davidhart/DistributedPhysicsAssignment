// David Hart - 2012

#pragma once

#include "Vector.h"
#include "ShapeBatch.h"

namespace Physics
{

class PhysicsObject
{

public:

	virtual void SetPosition(const Vector2d& position);
	virtual Vector2d GetPosition() const;

	void SetVelocity(const Vector2d& velocity);
	Vector2d GetVelocity() const;

private:

	Vector2d _position;
	Vector2d _velocity;
	double _mass;
	
};


class BoxObject : public PhysicsObject
{

public:
	
	BoxObject(Quad& quad);
	void SetPosition(const Vector2d& position);

private:

	Quad& _quad;

};


class TriangleObject : public PhysicsObject
{

public:

	TriangleObject(Triangle& triangle);
	void SetPosition(const Vector2d& position);

private:

	Triangle& _triangle;
};

}