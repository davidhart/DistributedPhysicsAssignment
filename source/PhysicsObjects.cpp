// David Hart - 2012

#include "PhysicsObjects.h"

using namespace Physics;

void PhysicsObject::SetPosition(const Vector2d& position)
{
	_position = position;
}

Vector2d PhysicsObject::GetPosition() const
{
	return _position;
}

void PhysicsObject::SetVelocity(const Vector2d& velocity)
{
	_velocity = velocity;
}

Vector2d PhysicsObject::GetVelocity() const
{
	return _velocity;
}


BoxObject::BoxObject(Quad& quad) : 
	_quad(quad)
{
}

void BoxObject::SetPosition(const Vector2d& position)
{
	_quad._position.x((float)position.x());
	_quad._position.y((float)position.y());

	PhysicsObject::SetPosition(position);
}

TriangleObject::TriangleObject(Triangle& triangle) :
	_triangle(triangle)
{
}

void TriangleObject::SetPosition(const Vector2d& position)
{
	Vector2f point((float)position.x(), (float)position.y());

	_triangle._points[0] = point;
	_triangle._points[1] = point + Vector2f(0.5, 1);
	_triangle._points[1] = point + Vector2f(1, 0);

	PhysicsObject::SetPosition(position);
}
