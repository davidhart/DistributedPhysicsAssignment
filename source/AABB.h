#pragma once

#include "Vector.h"

class AABB
{
public:
	
	AABB(const Vector2d& min, const Vector2d& max);

	bool Intersects(const AABB& aabb, double& penetrationDepth, Vector2d& contactNormal);
	bool Intersects(const Vector2d& b, double& penetrationDepth, Vector2d& contactNormal);

	Vector2d Midpoint() const;
	Vector2d Size() const;

private:

	Vector2d _min;
	Vector2d _max;

};