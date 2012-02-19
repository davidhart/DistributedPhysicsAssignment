// David Hart - 2012

#pragma once

#include "Util.h"
#include <iostream>
#include <cmath>
#include <cassert>

class Vector2
{

public:

	Vector2();
	explicit Vector2(float v);
	Vector2(float x, float y);

	Vector2& operator+=(const Vector2& rhs);
	Vector2& operator-=(const Vector2& rhs);
	Vector2& operator*=(const Vector2& rhs);
	Vector2& operator*=(float rhs);
	Vector2& operator/=(const Vector2& rhs);
	Vector2& operator/=(float rhs);
	Vector2 operator-() const;
	
	float length() const;
	float dot(const Vector2& rhs) const;
	Vector2 unit() const;
	const Vector2& normalize();

	float x() const;
	float y() const;

	void x(float x);
	void y(float y);

	float element(unsigned int i) const;
	void element(unsigned int i, float value);
	float operator[](unsigned int i) const;

private:

	float _v[2];

};

inline Vector2& Vector2::operator+=(const Vector2& rhs)
{
	x(x() + rhs.x());
	y(y() + rhs.y());
	return *this;
}

inline Vector2& Vector2::operator-=(const Vector2& rhs)
{
	x(x() - rhs.x());
	y(y() - rhs.y());
	return *this;
}

inline Vector2& Vector2::operator*=(const Vector2& rhs)
{
	x(x() * rhs.x());
	y(y() * rhs.y());
	return *this;
}

inline Vector2& Vector2::operator*=(float rhs)
{
	x(x() * rhs);
	y(y() * rhs);
	return *this;
}

inline Vector2& Vector2::operator/=(const Vector2& rhs)
{
	x(x() / rhs.x());
	y(y() / rhs.y());
	return *this;
}

inline Vector2& Vector2::operator/=(float rhs)
{
	x(x() / rhs);
	y(y() / rhs);
	return *this;
}

inline Vector2 Vector2::operator-() const
{
	return Vector2(-x(), -y());
}

inline float Vector2::length() const
{
	return sqrt(x() * x() + y() * y());
}

inline float Vector2::dot(const Vector2& rhs) const
{
	return x() * rhs.x() + y() * rhs.y();
}

inline Vector2 Vector2::unit() const
{
	return Vector2(*this).normalize();
}

inline const Vector2& Vector2::normalize()
{
	float l = length();
	if (Util::FloatEquality(l, 0))
		return *this = Vector2(0);
	return *this /= l;
}

inline float Vector2::element(unsigned int i) const
{
	assert(i < 2);
	return _v[i];
}

inline void Vector2::element(unsigned int i, float value)
{
	assert(i < 2);
	_v[i] = value;
}

inline float Vector2::operator[](unsigned int i) const
{
	return element(i);
}

inline float Vector2::x() const
{
	return _v[0];
}

inline float Vector2::y() const
{
	return _v[1];
}

inline void Vector2::x(float x)
{
	_v[0] = x;
}

inline void Vector2::y(float y)
{
	_v[1] = y;
}

inline Vector2 operator+(const Vector2& lhs, const Vector2& rhs)
{
	return Vector2(lhs) += rhs;
}

inline Vector2 operator-(const Vector2& lhs, const Vector2& rhs)
{
	return Vector2(lhs) -= rhs;
}

inline Vector2 operator*(const Vector2& lhs, const Vector2& rhs)
{
	return Vector2(lhs) *= rhs;
}

inline Vector2 operator*(float lhs, const Vector2& rhs)
{
	return Vector2(rhs) *= lhs;
}

inline Vector2 operator*(const Vector2& lhs, float rhs)
{
	return Vector2(lhs) *= rhs;
}

inline Vector2 operator/(const Vector2& lhs, const Vector2& rhs)
{
	return Vector2(lhs) /= rhs;
}

inline Vector2 operator/(float lhs, const Vector2& rhs)
{
	return Vector2(lhs / rhs.x(), lhs / rhs.y());
}

inline Vector2 operator/(const Vector2& lhs, float rhs)
{
	return Vector2(lhs) /= rhs;
}

inline bool operator==(const Vector2& lhs, const Vector2& rhs)
{
	return Util::FloatEquality(lhs.x(), rhs.y()) &&
		Util::FloatEquality(lhs.y(), rhs.y());
}

inline bool operator!=(const Vector2& lhs, const Vector2& rhs)
{
	return !(lhs == rhs);
}

inline std::ostream& operator<<(std::ostream& lhs, const Vector2& rhs)
{
	lhs << "v2(" << rhs.x() << ", " << rhs.y() << ")";
	return lhs;
}

inline std::istream& operator>>(std::istream& lhs, Vector2& rhs)
{
	char c;
	float x,y;
	lhs >> c >> c >> c >> x >> c >> y >> c;
	rhs = Vector2(x,y);

	return lhs;
}