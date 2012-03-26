// David Hart - 2011

#include "Matrix4.h"

Matrix4::Matrix4()
{

}

Matrix4::Matrix4(const Vector4f& c0, const Vector4f& c1, const Vector4f& c2, const Vector4f& c3)
{
	_m[0] = c0;
	_m[1] = c1;
	_m[2] = c2;
	_m[3] = c3;
}