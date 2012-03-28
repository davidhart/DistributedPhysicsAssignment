// David Hart - 2011

#pragma once

#include "Util.h"
#include <iostream>
#include <cmath>
#include <cassert>

template <typename T> 
Vector3<T>::Vector3()
{
	_v[0] = 0;
	_v[1] = 0;
	_v[2] = 0;
}

template <typename T> 
Vector3<T>::Vector3(T v)
{
	_v[0] = v;
	_v[1] = v;
	_v[2] = v;
}

template <typename T> template <typename R>
Vector3<T>::Vector3(const Vector3<R>& rhs)
{
	_v[0] = (T)rhs.x();
	_v[1] = (T)rhs.y();
	_v[2] = (T)rhs.z();
}

template <typename T> 
Vector3<T>::Vector3(T x, T y, T z)
{
	_v[0] = x;
	_v[1] = y;
	_v[2] = z;
}

template <typename T> 
inline Vector3<T>& Vector3<T>::operator+=(const Vector3<T>& rhs)
{
	x(x() + rhs.x());
	y(y() + rhs.y());
	z(z() + rhs.z());
	return *this;
}

template <typename T> 
inline Vector3<T>& Vector3<T>::operator-=(const Vector3<T>& rhs)
{
	x(x() - rhs.x());
	y(y() - rhs.y());
	z(z() - rhs.z());
	return *this;
}

template <typename T> 
inline Vector3<T>& Vector3<T>::operator*=(const Vector3<T>& rhs)
{
	x(x() * rhs.x());
	y(y() * rhs.y());
	z(z() * rhs.z());
	return *this;
}

template <typename T>
inline Vector3<T>& Vector3<T>::operator*=(T rhs)
{
	x(x() * rhs);
	y(y() * rhs);
	z(z() * rhs);
	return *this;
}

template <typename T> 
inline Vector3<T>& Vector3<T>::operator/=(const Vector3<T>& rhs)
{
	x(x() / rhs.x());
	y(y() / rhs.y());
	z(z() / rhs.z());
	return *this;
}

template <typename T>
inline Vector3<T>& Vector3<T>::operator/=(T rhs)
{
	x(x() / rhs);
	y(y() / rhs);
	z(z() / rhs);
	return *this;
}

template <typename T> 
inline Vector3<T> Vector3<T>::operator-() const
{
	return Vector3(-x(), -y(), -z());
}

template <typename T> 
inline T Vector3<T>::length() const
{
	return sqrt(x() * x() + y() * y() + z() * z());
}

template <typename T> 
inline T Vector3<T>::dot(const Vector3<T>& rhs) const
{
	return x() * rhs.x() + y() * rhs.y() + z() * rhs.z();
}

template <typename T> 
inline Vector3<T> Vector3<T>::cross(const Vector3<T>& rhs) const
{
	return Vector3<T>(y() * rhs.z() - z() * rhs.y(), z() * rhs.x() - x() * rhs.z(), x() * rhs.y() - y() * rhs.x());
}

template <typename T>
inline Vector3<T> Vector3<T>::unit() const
{
	return Vector3<T>(*this).normalize();
}

template <typename T> 
inline const Vector3<T>& Vector3<T>::normalize()
{
	T l = length();
	if (Util::FloatEquality(l, 0))
		return *this = Vector3(0);
	return *this /= l;
}

template <typename T>
inline T Vector3<T>::element(unsigned int i) const
{
	assert(i < 3);
	return _v[i];
}

template <typename T>
inline void Vector3<T>::element(unsigned int i, T value)
{
	assert(i < 3);
	_v[i] = value;
}

template <typename T> 
inline T Vector3<T>::operator[](unsigned int i) const
{
	return element(i);
}

template <typename T> 
inline T Vector3<T>::x() const
{
	return _v[0];
}

template <typename T> 
inline T Vector3<T>::y() const
{
	return _v[1];
}

template <typename T> 
inline T Vector3<T>::z() const
{
	return _v[2];
}

template <typename T> 
inline void Vector3<T>::x(T x)
{
	_v[0] = x;
}

template <typename T> 
inline void Vector3<T>::y(T y)
{
	_v[1] = y;
}

template <typename T> 
inline void Vector3<T>::z(T z)
{
	_v[2] = z;
}

template <typename T>
inline Vector4<T> Vector3<T>::xyz0()
{
	return Vector4<T>(x(), y(), z(), (T)0);
}

template <typename T>
inline Vector4<T> Vector3<T>::xyz1()
{
	return Vector4<T>(x(), y(), z(), (T)1);
}


template <typename T> 
inline Vector3<T> operator+(const Vector3<T>& lhs, const Vector3<T> & rhs)
{
	return Vector3(lhs) += rhs;
}

template <typename T> 
inline Vector3<T> operator-(const Vector3<T>& lhs, const Vector3<T>& rhs)
{
	return Vector3<T>(lhs) -= rhs;
}

template <typename T> 
inline Vector3<T> operator*(const Vector3<T>& lhs, const Vector3<T>& rhs)
{
	return Vector3<T>(lhs) *= rhs;
}

template <typename T> 
inline Vector3<T> operator*(T lhs, const Vector3<T>& rhs)
{
	return Vector3<T>(rhs) *= lhs;
}

template <typename T> 
inline Vector3<T> operator*(const Vector3<T>& lhs, T rhs)
{
	return Vector3<T>(lhs) *= rhs;
}

template <typename T> 
inline Vector3<T> operator/(const Vector3<T>& lhs, const Vector3<T>& rhs)
{
	return Vector3<T>(lhs) /= rhs;
}

template <typename T> 
inline Vector3<T> operator/(T lhs, const Vector3<T>& rhs)
{
	return Vector3<T>(lhs / rhs.x(), lhs / rhs.y(), lhs / rhs.z());
}

template <typename T> 
inline Vector3<T> operator/(const Vector3<T>& lhs, T rhs)
{
	return Vector3<T>(lhs) /= rhs;
}
 
template <typename T> 
inline bool operator==(const Vector3<T>& lhs, const Vector3<T>& rhs)
{
	return Util::FloatEquality(lhs.x(), rhs.y()) &&
		Util::FloatEquality(lhs.y(), rhs.y()) &&
		Util::FloatEquality(lhs.z(), rhs.z());
}

template <typename T> 
inline bool operator!=(const Vector3<T>& lhs, const Vector3<T>& rhs)
{
	return !(lhs == rhs);
}

template <typename T>
inline std::ostream& operator<<(std::ostream& lhs, const Vector3<T>& rhs)
{
	lhs << "v3(" << rhs.x() << ", " << rhs.y() << ", " << rhs.z() << ")";
	return lhs;
}

template <typename T>
inline std::istream& operator>>(std::istream& lhs, Vector3<T>& rhs)
{
	char c;
	T x,y,z;
	lhs >> c >> c >> c >> x >> c >> y >> c >> z >> c;
	rhs = Vector3<T>(x,y,z);

	return lhs;
}