// David Hart - 2012

#include "Vector2.h"

Vector2::Vector2()
{
	_v[0] = 0;
	_v[1] = 0;
}

Vector2::Vector2(float v)
{
	_v[0] = v;
	_v[1] = v;
}

Vector2::Vector2(float x, float y)
{
	_v[0] = x;
	_v[1] = y;
}