// David Hart - 2011

#pragma once

#include "Util.h"
#include <iostream>
#include <cmath>
#include <cassert>

class Vector3
{

public:

	Vector3();
	explicit Vector3(float v);
	Vector3(float x, float y, float z);

	Vector3& operator+=(const Vector3& rhs);
	Vector3& operator-=(const Vector3& rhs);
	Vector3& operator*=(const Vector3& rhs);
	Vector3& operator*=(float rhs);
	Vector3& operator/=(const Vector3& rhs);
	Vector3& operator/=(float rhs);
	Vector3 operator-() const;
	
	float length() const;
	float dot(const Vector3& rhs) const;
	Vector3 cross(const Vector3& rhs) const;
	Vector3 unit() const;
	const Vector3& normalize();

	float x() const;
	float y() const;
	float z() const;

	void x(float x);
	void y(float y);
	void z(float z);

	float element(unsigned int i) const;
	void element(unsigned int i, float value);
	float operator[](unsigned int i) const;

private:

	float _v[3];

};

inline Vector3& Vector3::operator+=(const Vector3& rhs)
{
	x(x() + rhs.x());
	y(y() + rhs.y());
	z(z() + rhs.z());
	return *this;
}

inline Vector3& Vector3::operator-=(const Vector3& rhs)
{
	x(x() - rhs.x());
	y(y() - rhs.y());
	z(z() - rhs.z());
	return *this;
}

inline Vector3& Vector3::operator*=(const Vector3& rhs)
{
	x(x() * rhs.x());
	y(y() * rhs.y());
	z(z() * rhs.z());
	return *this;
}

inline Vector3& Vector3::operator*=(float rhs)
{
	x(x() * rhs);
	y(y() * rhs);
	z(z() * rhs);
	return *this;
}

inline Vector3& Vector3::operator/=(const Vector3& rhs)
{
	x(x() / rhs.x());
	y(y() / rhs.y());
	z(z() / rhs.z());
	return *this;
}

inline Vector3& Vector3::operator/=(float rhs)
{
	x(x() / rhs);
	y(y() / rhs);
	z(z() / rhs);
	return *this;
}

inline Vector3 Vector3::operator-() const
{
	return Vector3(-x(), -y(), -z());
}

inline float Vector3::length() const
{
	return sqrt(x() * x() + y() * y() + z() * z());
}

inline float Vector3::dot(const Vector3& rhs) const
{
	return x() * rhs.x() + y() * rhs.y() + z() * rhs.z();
}

inline Vector3 Vector3::cross(const Vector3& rhs) const
{
	return Vector3(y() * rhs.z() - z() * rhs.y(), z() * rhs.x() - x() * rhs.z(), x() * rhs.y() - y() * rhs.x());
}

inline Vector3 Vector3::unit() const
{
	return Vector3(*this).normalize();
}

inline const Vector3& Vector3::normalize()
{
	float l = length();
	if (Util::FloatEquality(l, 0))
		return *this = Vector3(0);
	return *this /= l;
}

inline float Vector3::element(unsigned int i) const
{
	assert(i < 3);
	return _v[i];
}

inline void Vector3::element(unsigned int i, float value)
{
	assert(i < 3);
	_v[i] = value;
}

inline float Vector3::operator[](unsigned int i) const
{
	return element(i);
}

inline float Vector3::x() const
{
	return _v[0];
}

inline float Vector3::y() const
{
	return _v[1];
}

inline float Vector3::z() const
{
	return _v[2];
}

inline void Vector3::x(float x)
{
	_v[0] = x;
}

inline void Vector3::y(float y)
{
	_v[1] = y;
}

inline void Vector3::z(float z)
{
	_v[2] = z;
}

inline Vector3 operator+(const Vector3& lhs, const Vector3& rhs)
{
	return Vector3(lhs) += rhs;
}

inline Vector3 operator-(const Vector3& lhs, const Vector3& rhs)
{
	return Vector3(lhs) -= rhs;
}

inline Vector3 operator*(const Vector3& lhs, const Vector3& rhs)
{
	return Vector3(lhs) *= rhs;
}

inline Vector3 operator*(float lhs, const Vector3& rhs)
{
	return Vector3(rhs) *= lhs;
}

inline Vector3 operator*(const Vector3& lhs, float rhs)
{
	return Vector3(lhs) *= rhs;
}

inline Vector3 operator/(const Vector3& lhs, const Vector3& rhs)
{
	return Vector3(lhs) /= rhs;
}

inline Vector3 operator/(float lhs, const Vector3& rhs)
{
	return Vector3(lhs / rhs.x(), lhs / rhs.y(), lhs / rhs.z());
}

inline Vector3 operator/(const Vector3& lhs, float rhs)
{
	return Vector3(lhs) /= rhs;
}

inline bool operator==(const Vector3& lhs, const Vector3& rhs)
{
	return Util::FloatEquality(lhs.x(), rhs.y()) &&
		Util::FloatEquality(lhs.y(), rhs.y()) &&
		Util::FloatEquality(lhs.z(), rhs.z());
}

inline bool operator!=(const Vector3& lhs, const Vector3& rhs)
{
	return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& lhs, const Vector3& rhs)
{
	lhs << "v3(" << rhs.x() << ", " << rhs.y() << ", " << rhs.z() << ")";
	return lhs;
}

inline std::istream& operator>>(std::istream& lhs, Vector3& rhs)
{
	char c;
	float x,y,z;
	lhs >> c >> c >> c >> x >> c >> y >> c >> z >> c;
	rhs = Vector3(x,y,z);

	return lhs;
}