// David Hart - 2012

#pragma once

#include "Util.h"
#include <iostream>
#include <cmath>
#include <cassert>

template <typename T>
Vector2<T>::Vector2()
{
	_v[0] = 0;
	_v[1] = 0;
}

template <typename T>
Vector2<T>::Vector2(T v)
{
	_v[0] = v;
	_v[1] = v;
}

template <typename T>
Vector2<T>::Vector2(T x, T y)
{
	_v[0] = x;
	_v[1] = y;
}

template <typename T>
inline Vector2<T>& Vector2<T>::operator+=(const Vector2<T>& rhs)
{
	x(x() + rhs.x());
	y(y() + rhs.y());
	return *this;
}

template <typename T>
inline Vector2<T>& Vector2<T>::operator-=(const Vector2<T>& rhs)
{
	x(x() - rhs.x());
	y(y() - rhs.y());
	return *this;
}

template <typename T>
inline Vector2<T>& Vector2<T>::operator*=(const Vector2<T>& rhs)
{
	x(x() * rhs.x());
	y(y() * rhs.y());
	return *this;
}

template <typename T>
inline Vector2<T>& Vector2<T>::operator*=(T rhs)
{
	x(x() * rhs);
	y(y() * rhs);
	return *this;
}

template <typename T>
inline Vector2<T>& Vector2<T>::operator/=(const Vector2<T>& rhs)
{
	x(x() / rhs.x());
	y(y() / rhs.y());
	return *this;
}

template <typename T>
inline Vector2<T>& Vector2<T>::operator/=(T rhs)
{
	x(x() / rhs);
	y(y() / rhs);
	return *this;
}

template <typename T>
inline Vector2<T> Vector2<T>::operator-() const
{
	return Vector2<T>(-x(), -y());
}

template <typename T>
inline T Vector2<T>::length() const
{
	return sqrt(x() * x() + y() * y());
}

template <typename T>
inline T Vector2<T>::dot(const Vector2<T>& rhs) const
{
	return x() * rhs.x() + y() * rhs.y();
}

template <typename T>
inline Vector2<T> Vector2<T>::unit() const
{
	return Vector2<T>(*this).normalize();
}

template <typename T>
inline const Vector2<T>& Vector2<T>::normalize()
{
	T l = length();
	if (Util::FloatEquality(l, 0))
		return *this = Vector2(0);
	return *this /= l;
}

template <typename T>
inline T Vector2<T>::element(unsigned int i) const
{
	assert(i < 2);
	return _v[i];
}

template <typename T>
inline void Vector2<T>::element(unsigned int i, T value)
{
	assert(i < 2);
	_v[i] = value;
}

template <typename T>
inline T Vector2<T>::operator[](unsigned int i) const
{
	return element(i);
}

template <typename T>
inline T Vector2<T>::x() const
{
	return _v[0];
}

template <typename T>
inline T Vector2<T>::y() const
{
	return _v[1];
}

template <typename T>
inline void Vector2<T>::x(T x)
{
	_v[0] = x;
}

template <typename T>
inline void Vector2<T>::y(T y)
{
	_v[1] = y;
}

template <typename T>
inline Vector2<T> operator+(const Vector2<T>& lhs, const Vector2<T>& rhs)
{
	return Vector2<T>(lhs) += rhs;
}

template <typename T>
inline Vector2<T> operator-(const Vector2<T>& lhs, const Vector2<T>& rhs)
{
	return Vector2<T>(lhs) -= rhs;
}

template <typename T>
inline Vector2<T> operator*(const Vector2<T>& lhs, const Vector2<T>& rhs)
{
	return Vector2<T>(lhs) *= rhs;
}

template <typename T>
inline Vector2<T> operator*(T lhs, const Vector2<T>& rhs)
{
	return Vector2<T>(rhs) *= lhs;
}

template <typename T>
inline Vector2<T> operator*(const Vector2<T>& lhs, T rhs)
{
	return Vector2<T>(lhs) *= rhs;
}

template <typename T>
inline Vector2<T> operator/(const Vector2<T>& lhs, const Vector2<T>& rhs)
{
	return Vector2<T>(lhs) /= rhs;
}

template <typename T>
inline Vector2<T> operator/(T lhs, const Vector2<T>& rhs)
{
	return Vector2<T>(lhs / rhs.x(), lhs / rhs.y());
}

template <typename T>
inline Vector2<T> operator/(const Vector2<T>& lhs, T rhs)
{
	return Vector2<T>(lhs) /= rhs;
}

template <typename T>
inline bool operator==(const Vector2<T>& lhs, const Vector2<T>& rhs)
{
	return Util::FloatEquality(lhs.x(), rhs.y()) &&
		Util::FloatEquality(lhs.y(), rhs.y());
}

template <typename T>
inline bool operator!=(const Vector2<T>& lhs, const Vector2<T>& rhs)
{
	return !(lhs == rhs);
}

template <typename T>
inline std::ostream& operator<<(std::ostream& lhs, const Vector2<T>& rhs)
{
	lhs << "v2(" << rhs.x() << ", " << rhs.y() << ")";
	return lhs;
}

template <typename T>
inline std::istream& operator>>(std::istream& lhs, Vector2<T>& rhs)
{
	char c;
	T x,y;
	lhs >> c >> c >> c >> x >> c >> y >> c;
	rhs = Vector2<T>(x,y);

	return lhs;
}
