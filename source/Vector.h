#pragma once

template <typename T> class Vector2;
template <typename T> class Vector3;
template <typename T> class Vector4;

template <typename T>
class Vector2
{

public:

	Vector2();
	explicit Vector2(T v);
	template <typename R> explicit Vector2(const Vector2<R>& rhs);
	Vector2(T x, T y);

	Vector2<T>& operator+=(const Vector2<T>& rhs);
	Vector2<T>& operator-=(const Vector2<T>& rhs);
	Vector2<T>& operator*=(const Vector2<T>& rhs);
	Vector2<T>& operator*=(T rhs);
	Vector2<T>& operator/=(const Vector2<T>& rhs);
	Vector2<T>& operator/=(T rhs);
	Vector2<T> operator-() const;
	
	T length() const;
	T dot(const Vector2<T>& rhs) const;
	Vector2<T> unit() const;
	Vector2<T> tangent() const;
	const Vector2<T>& normalize();

	T x() const;
	T y() const;

	void x(T x);
	void y(T y);

	Vector3<T> xy0();
	Vector3<T> xy1();

	T element(unsigned int i) const;
	void element(unsigned int i, T value);
	T operator[](unsigned int i) const;

private:

	T _v[2];

};

#include "Vector2.inl"

typedef Vector2<float> Vector2f;
typedef Vector2<double> Vector2d;
typedef Vector2<int> Vector2i;

template <typename T>
class Vector3
{

public:

	Vector3();
	explicit Vector3(T v);
	template <typename R> explicit Vector3(const Vector3<R>& rhs);
	Vector3(T x, T y, T z);

	Vector3<T>& operator+=(const Vector3<T>& rhs);
	Vector3<T>& operator-=(const Vector3<T>& rhs);
	Vector3<T>& operator*=(const Vector3<T>& rhs);
	Vector3<T>& operator*=(T rhs);
	Vector3<T>& operator/=(const Vector3<T>& rhs);
	Vector3<T>& operator/=(T rhs);
	Vector3<T> operator-() const;
	
	T length() const;
	T dot(const Vector3<T>& rhs) const;
	Vector3<T> cross(const Vector3<T>& rhs) const;
	Vector3<T> unit() const;
	const Vector3<T>& normalize();

	T x() const;
	T y() const;
	T z() const;

	void x(T x);
	void y(T y);
	void z(T z);

	Vector4<T> xyz1();
	Vector4<T> xyz0();

	T element(unsigned int i) const;
	void element(unsigned int i, T value);
	T operator[](unsigned int i) const;

private:

	T _v[3];

};

#include "Vector3.inl"

typedef Vector3<float> Vector3f;
typedef Vector3<double> Vector3d;
typedef Vector3<int> Vector3i;

template <typename T>
class Vector4
{

public:

	Vector4();
	explicit Vector4(T v);
	template <typename R> explicit Vector4(const Vector4<R>& rhs);
	Vector4(const Vector3<T>& v, T w);
	Vector4(T x, T y, T z, T w);

	Vector4<T>& operator+=(const Vector4<T>& rhs);
	Vector4<T>& operator-=(const Vector4<T>& rhs);
	Vector4<T>& operator*=(const Vector4<T>& rhs);
	Vector4<T>& operator*=(T rhs);
	Vector4<T>& operator/=(const Vector4<T>& rhs);
	Vector4<T>& operator/=(T rhs);
	Vector4<T> operator-() const;
	
	T length() const;
	T dot(const Vector4<T>& rhs) const;

	T x() const;
	T y() const;
	T z() const;
	T w() const;

	void x(T x);
	void y(T y);
	void z(T z);
	void w(T w);

	T element(unsigned int i) const;
	void element(unsigned int i, T value);
	T operator[](unsigned int i) const;

private:

	T _v[4];

};

#include "Vector4.inl"

typedef Vector4<float> Vector4f;
typedef Vector4<double> Vector4d;
typedef Vector4<int> Vector4i;
