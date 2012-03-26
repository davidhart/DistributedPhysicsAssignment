// David Hart - 2011

#pragma once

#include "Util.h"
#include <cmath>
#include <cassert>

template <typename T>
Vector4<T>::Vector4()
{
	_v[0] = 0;
	_v[1] = 0;
	_v[2] = 0;
	_v[3] = 0;
}

template <typename T>
Vector4<T>::Vector4(T v)
{
	_v[0] = v;
	_v[1] = v;
	_v[2] = v;
	_v[3] = v;
}

template <typename T>
Vector4<T>::Vector4(const Vector3<T>& v, T w)
{
	_v[0] = v.x();
	_v[1] = v.y();
	_v[2] = v.z();
	_v[3] = w;
}

template <typename T>
Vector4<T>::Vector4(T x, T y, T z, T w)
{
	_v[0] = x;
	_v[1] = y;
	_v[2] = z;
	_v[3] = w;
}

template <typename T>
inline Vector4<T>& Vector4<T>::operator+=(const Vector4<T>& rhs)
{
	x(x() + rhs.x());
	y(y() + rhs.y());
	z(z() + rhs.z());
	w(w() + rhs.w());
	return *this;
}

template <typename T>
inline Vector4<T>& Vector4<T>::operator-=(const Vector4<T>& rhs)
{
	x(x() - rhs.x());
	y(y() - rhs.y());
	z(z() - rhs.z());
	w(w() - rhs.w());
	return *this;
}

template <typename T>
inline Vector4<T>& Vector4<T>::operator*=(const Vector4<T>& rhs)
{
	x(x() * rhs.x());
	y(y() * rhs.y());
	z(z() * rhs.z());
	w(w() * rhs.w());
	return *this;
}

template <typename T>
inline Vector4<T>& Vector4<T>::operator*=(T rhs)
{
	x(x() * rhs);
	y(y() * rhs);
	z(z() * rhs);
	w(w() * rhs);
	return *this;
}

template <typename T>
inline Vector4<T>& Vector4<T>::operator/=(const Vector4<T>& rhs)
{
	x(x() / rhs.x());
	y(y() / rhs.y());
	z(z() / rhs.z());
	w(w() / rhs.w());
	return *this;
}

template <typename T>
inline Vector4<T>& Vector4<T>::operator/=(T rhs)
{
	x(x() / rhs);
	y(y() / rhs);
	z(z() / rhs);
	w(w() / rhs);
	return *this;
}

template <typename T>
inline Vector4<T> Vector4<T>::operator-() const
{
	return Vector4<T>(-x(), -y(), -z(), - w());
}

template <typename T>
inline T Vector4<T>::length() const
{
	return sqrt(x() * x() + y() * y() + z() * z() + w() + w());
}

template <typename T>
inline T Vector4<T>::dot(const Vector4<T>& rhs) const
{
	return x() * rhs.x() + y() * rhs.y() + z() * rhs.z() + w() * rhs.w();
}

template <typename T>
inline T Vector4<T>::element(unsigned int i) const
{
	assert(i < 4);
	return _v[i];
}

template <typename T>
inline void Vector4<T>::element(unsigned int i, T value)
{
	assert(i < 4);
	_v[i] = value;
}

template <typename T>
inline T Vector4<T>::operator[](unsigned int i) const
{
	return element(i);
}

template <typename T>
inline T Vector4<T>::x() const
{
	return _v[0];
}

template <typename T>
inline T Vector4<T>::y() const
{
	return _v[1];
}

template <typename T>
inline T Vector4<T>::z() const
{
	return _v[2];
}

template <typename T>
inline T Vector4<T>::w() const
{
	return _v[3];
}

template <typename T>
inline void Vector4<T>::x(T x)
{
	_v[0] = x;
}

template <typename T>
inline void Vector4<T>::y(T y)
{
	_v[1] = y;
}

template <typename T>
inline void Vector4<T>::z(T z)
{
	_v[2] = z;
}

template <typename T>
inline void Vector4<T>::w(T w)
{
	_v[3] = w;
}

template <typename T>
inline Vector4<T> operator+(const Vector4<T>& lhs, const Vector4<T>& rhs)
{
	return Vector4<T>(lhs) += rhs;
}

template <typename T>
inline Vector4<T> operator-(const Vector4<T>& lhs, const Vector4<T>& rhs)
{
	return Vector4<T>(lhs) -= rhs;
}

template <typename T>
inline Vector4<T> operator*(const Vector4<T>& lhs, const Vector4<T>& rhs)
{
	return Vector4<T>(lhs) *= rhs;
}

template <typename T>
inline Vector4<T> operator*(T lhs, const Vector4<T>& rhs)
{
	return Vector4<T>(rhs) *= lhs;
}

template <typename T>
inline Vector4<T> operator*(const Vector4<T>& lhs, T rhs)
{
	return Vector4<T>(lhs) *= rhs;
}

template <typename T>
inline Vector4<T> operator/(const Vector4<T>& lhs, const Vector4<T>& rhs)
{
	return Vector4<T>(lhs) /= rhs;
}

template <typename T>
inline Vector4<T> operator/(T lhs, const Vector4<T>& rhs)
{
	return Vector4<T>(lhs / rhs.x(), lhs / rhs.y(), lhs / rhs.z(), lhs / rhs.w());
}

template <typename T>
inline Vector4<T> operator/(const Vector4<T>& lhs, T rhs)
{
	return Vector4(lhs) /= rhs;
}

template <typename T>
inline bool operator==(const Vector4<T>& lhs, const Vector4<T>& rhs)
{
	return Util::FloatEquality(lhs.x(), rhs.y()) &&
		Util::FloatEquality(lhs.y(), rhs.y()) &&
		Util::FloatEquality(lhs.z(), rhs.z()) &&
		Util::FloatEquality(lhs.w(), rhs.w());
}

template <typename T>
inline bool operator!=(const Vector4<T>& lhs, const Vector4<T>& rhs)
{
	return !(lhs == rhs);
}

template <typename T>
inline std::ostream& operator<<(std::ostream& lhs, const Vector4<T>& rhs)
{
	lhs << "v4(" << rhs.x() << ", " << rhs.y() << ", ";
	lhs << rhs.z() << ", " << rhs.w() << ")";
	return lhs;
}

template <typename T>
inline std::istream& operator>>(std::istream& lhs, Vector4<T>& rhs)
{
	char c;
	T x,y,z, w;
	lhs >> c >> c >> c >> x >> c >> y >> c >> z >> c >> w >> c;
	rhs = Vector4<T>(x,y,z,w);

	return lhs;
}
