// David Hart - 2011

#include "Vector4.h"

Vector4::Vector4()
{
	_v[0] = 0;
	_v[1] = 0;
	_v[2] = 0;
	_v[3] = 0;
}

Vector4::Vector4(float v)
{
	_v[0] = v;
	_v[1] = v;
	_v[2] = v;
	_v[3] = v;
}

Vector4::Vector4(const Vector3& v, float w)
{
	_v[0] = v.x();
	_v[1] = v.y();
	_v[2] = v.z();
	_v[3] = w;
}

Vector4::Vector4(float x, float y, float z, float w)
{
	_v[0] = x;
	_v[1] = y;
	_v[2] = z;
	_v[3] = w;
}