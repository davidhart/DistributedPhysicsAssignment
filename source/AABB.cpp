#include "AABB.h"

AABB::AABB()
{
}

AABB::AABB(const Vector2d& min, const Vector2d& max) :
	_min(min),
	_max(max)
{
}

Vector2d AABB::Midpoint() const
{
	return (_min + _max) / 2.0;
}

Vector2d AABB::Size() const
{
	return _max - _min;
}

bool AABB::Intersects(const AABB& aabb, double& penetrationDistance, Vector2d& contactNormal)
{
	Vector2d dist = Midpoint() - aabb.Midpoint();

	if (dist.length() == 0)
		return false;
	
	Vector2d minSize((Size().x() + aabb.Size().x()) / 2, (Size().y() + aabb.Size().y()) / 2);

	if (abs(dist.x()) < minSize.x() && abs(dist.y()) < minSize.y())
	{

		if (abs(dist.x()) < abs(dist.y()))
		{
			dist.x(0);
			penetrationDistance = (minSize.y() - abs(dist.y())) / 2;
		}
		else
		{
			dist.y(0);
			penetrationDistance = (minSize.x() - abs(dist.x())) / 2;
		}

		contactNormal = dist.normalize();


		return true;
	}


	return false;
}

bool AABB::Intersects(const Vector2d& point, double& penetrationDistance, Vector2d& contactNormal)
{
	Vector2d dist = Midpoint() - point;

	if (dist.length() == 0)
		return false;
	
	Vector2d minSize(Size().x() / 2, Size().y() / 2);

	if (abs(dist.x()) < minSize.x() && abs(dist.y()) < minSize.y())
	{

		if (abs(dist.x()) < abs(dist.y()))
		{
			dist.x(0);
			penetrationDistance = (minSize.y() - abs(dist.y())) / 2;
		}
		else
		{
			dist.y(0);
			penetrationDistance = (minSize.x() - abs(dist.x())) / 2;
		}

		contactNormal = dist.normalize();


		return true;
	}


	return false;
}