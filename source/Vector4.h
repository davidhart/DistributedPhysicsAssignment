// David Hart - 2011

#pragma once

#include "Util.h"
#include "Vector3.h"
#include <cmath>
#include <cassert>

class Vector4
{

public:

	Vector4();
	explicit Vector4(float v);
	Vector4(const Vector3& v, float w);
	Vector4(float x, float y, float z, float w);

	Vector4& operator+=(const Vector4& rhs);
	Vector4& operator-=(const Vector4& rhs);
	Vector4& operator*=(const Vector4& rhs);
	Vector4& operator*=(float rhs);
	Vector4& operator/=(const Vector4& rhs);
	Vector4& operator/=(float rhs);
	Vector4 operator-() const;
	
	float length() const;
	float dot(const Vector4& rhs) const;

	float x() const;
	float y() const;
	float z() const;
	float w() const;

	void x(float x);
	void y(float y);
	void z(float z);
	void w(float w);

	float element(unsigned int i) const;
	void element(unsigned int i, float value);
	float operator[](unsigned int i) const;

private:

	float _v[4];

};

inline Vector4& Vector4::operator+=(const Vector4& rhs)
{
	x(x() + rhs.x());
	y(y() + rhs.y());
	z(z() + rhs.z());
	w(w() + rhs.w());
	return *this;
}

inline Vector4& Vector4::operator-=(const Vector4& rhs)
{
	x(x() - rhs.x());
	y(y() - rhs.y());
	z(z() - rhs.z());
	w(w() - rhs.w());
	return *this;
}

inline Vector4& Vector4::operator*=(const Vector4& rhs)
{
	x(x() * rhs.x());
	y(y() * rhs.y());
	z(z() * rhs.z());
	w(w() * rhs.w());
	return *this;
}

inline Vector4& Vector4::operator*=(float rhs)
{
	x(x() * rhs);
	y(y() * rhs);
	z(z() * rhs);
	w(w() * rhs);
	return *this;
}


inline Vector4& Vector4::operator/=(const Vector4& rhs)
{
	x(x() / rhs.x());
	y(y() / rhs.y());
	z(z() / rhs.z());
	w(w() / rhs.w());
	return *this;
}

inline Vector4& Vector4::operator/=(float rhs)
{
	x(x() / rhs);
	y(y() / rhs);
	z(z() / rhs);
	w(w() / rhs);
	return *this;
}

inline Vector4 Vector4::operator-() const
{
	return Vector4(-x(), -y(), -z(), - w());
}

inline float Vector4::length() const
{
	return sqrt(x() * x() + y() * y() + z() * z() + w() + w());
}

inline float Vector4::dot(const Vector4& rhs) const
{
	return x() * rhs.x() + y() * rhs.y() + z() * rhs.z() + w() * rhs.w();
}

inline float Vector4::element(unsigned int i) const
{
	assert(i < 4);
	return _v[i];
}

inline void Vector4::element(unsigned int i, float value)
{
	assert(i < 4);
	_v[i] = value;
}

inline float Vector4::operator[](unsigned int i) const
{
	return element(i);
}

inline float Vector4::x() const
{
	return _v[0];
}

inline float Vector4::y() const
{
	return _v[1];
}

inline float Vector4::z() const
{
	return _v[2];
}

inline float Vector4::w() const
{
	return _v[3];
}

inline void Vector4::x(float x)
{
	_v[0] = x;
}

inline void Vector4::y(float y)
{
	_v[1] = y;
}

inline void Vector4::z(float z)
{
	_v[2] = z;
}

inline void Vector4::w(float w)
{
	_v[3] = w;
}

inline Vector4 operator+(const Vector4& lhs, const Vector4& rhs)
{
	return Vector4(lhs) += rhs;
}

inline Vector4 operator-(const Vector4& lhs, const Vector4& rhs)
{
	return Vector4(lhs) -= rhs;
}

inline Vector4 operator*(const Vector4& lhs, const Vector4& rhs)
{
	return Vector4(lhs) *= rhs;
}

inline Vector4 operator*(float lhs, const Vector4& rhs)
{
	return Vector4(rhs) *= lhs;
}

inline Vector4 operator*(const Vector4& lhs, float rhs)
{
	return Vector4(lhs) *= rhs;
}

inline Vector4 operator/(const Vector4& lhs, const Vector4& rhs)
{
	return Vector4(lhs) /= rhs;
}

inline Vector4 operator/(float lhs, const Vector4& rhs)
{
	return Vector4(lhs / rhs.x(), lhs / rhs.y(), lhs / rhs.z(), lhs / rhs.w());
}

inline Vector4 operator/(const Vector4& lhs, float rhs)
{
	return Vector4(lhs) /= rhs;
}

inline bool operator==(const Vector4& lhs, const Vector4& rhs)
{
	return Util::FloatEquality(lhs.x(), rhs.y()) &&
		Util::FloatEquality(lhs.y(), rhs.y()) &&
		Util::FloatEquality(lhs.z(), rhs.z()) &&
		Util::FloatEquality(lhs.w(), rhs.w());
}

inline bool operator!=(const Vector4& lhs, const Vector4& rhs)
{
	return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& lhs, const Vector4& rhs)
{
	lhs << "v4(" << rhs.x() << ", " << rhs.y() << ", ";
	lhs << rhs.z() << ", " << rhs.w() << ")";
	return lhs;
}

inline std::istream& operator>>(std::istream& lhs, Vector4& rhs)
{
	char c;
	float x,y,z, w;
	lhs >> c >> c >> c >> x >> c >> y >> c >> z >> c >> w >> c;
	rhs = Vector4(x,y,z,w);

	return lhs;
}