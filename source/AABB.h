// David Hart - 2012

#pragma once

#include "Vector.h"

class AABB
{
public:
	
	AABB();
	AABB(const Vector2d& min, const Vector2d& max);

	bool Intersects(const AABB& aabb, double& penetrationDepth, Vector2d& contactNormal);
	bool Intersects(const Vector2d& b, double& penetrationDepth, Vector2d& contactNormal);

	Vector2d Midpoint() const;
	Vector2d Size() const;

	const Vector2d& Min() const;
	const Vector2d& Max() const;

private:

	Vector2d _min;
	Vector2d _max;

};

inline const Vector2d& AABB::Min() const
{
	return _min;
}

inline const Vector2d& AABB::Max() const
{
	return _max;
}