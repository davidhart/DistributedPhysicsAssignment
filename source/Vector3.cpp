// David Hart - 2011

#include "Vector3.h"

Vector3::Vector3()
{
	_v[0] = 0;
	_v[1] = 0;
	_v[2] = 0;
}

Vector3::Vector3(float v)
{
	_v[0] = v;
	_v[1] = v;
	_v[2] = v;
}

Vector3::Vector3(float x, float y, float z)
{
	_v[0] = x;
	_v[1] = y;
	_v[2] = z;
}