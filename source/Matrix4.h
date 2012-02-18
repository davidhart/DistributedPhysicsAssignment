// David Hart - 2011

#pragma once

#include "Vector4.h"
#include "Maths.h"

class Matrix4
{

public:

	Matrix4();
	Matrix4(const Vector4& c0, const Vector4& c1, const Vector4& c2, const Vector4& c3);

	Matrix4 operator*(const Matrix4& rhs) const;
	Matrix4& operator*= (const Matrix4& rhs);

	void cell(unsigned int column, unsigned int row, float value);
	float cell(unsigned int column, unsigned int row) const;

	void column(unsigned int column, const Vector4& value);
	const Vector4& column(unsigned int column) const;

	void row(unsigned int row, const Vector4& value);
	Vector4 row(unsigned int row) const;

	void Transform(Vector4& inout) const;

	static void Identity(Matrix4& out);
	static void Translation(Matrix4& out, const Vector3& translation);
	static void PerspectiveFov(Matrix4& out, float fov, float aspect, float nearclip, float farclip);
	static void LookAt(Matrix4& out, const Vector3& eye, const Vector3& at, const Vector3& up);
	static void Scale(Matrix4& out, float scale);
	static void Scale(Matrix4& out, const Vector3& scale);
	static void RotationAxis(Matrix4& out, const Vector3& axis, float angle);
	
private:

	Vector4 _m[4];

};

inline Matrix4& Matrix4::operator*=(const Matrix4& rhs)
{
	*this = Matrix4( Vector4(row(0).dot(rhs.column(0)),
							row(1).dot(rhs.column(0)),
							row(2).dot(rhs.column(0)),
							row(3).dot(rhs.column(0))),

					Vector4(row(0).dot(rhs.column(1)),
							row(1).dot(rhs.column(1)),
							row(2).dot(rhs.column(1)),
							row(3).dot(rhs.column(1))),

					Vector4(row(0).dot(rhs.column(2)),
							row(1).dot(rhs.column(2)),
							row(2).dot(rhs.column(2)),
							row(3).dot(rhs.column(2))),

					Vector4(row(0).dot(rhs.column(3)),
							row(1).dot(rhs.column(3)),
							row(2).dot(rhs.column(3)),
							row(3).dot(rhs.column(3)))
				   );

	return *this;
}

inline void Matrix4::cell(unsigned int column, unsigned int row, float value)
{
	_m[column].element(row, value);
}

inline float Matrix4::cell(unsigned int column, unsigned int row) const
{
	return _m[column].element(row);
}

inline void Matrix4::column(unsigned int column, const Vector4& value)
{
	assert(column < 4);

	_m[column] = value;
}

inline const Vector4& Matrix4::column(unsigned int column) const
{
	assert(column < 4);

	return _m[column];
}

inline void Matrix4::row(unsigned int row, const Vector4& value)
{
	assert(row < 4);

	_m[0].element(row, value[0]);
	_m[1].element(row, value[1]);
	_m[2].element(row, value[2]);
	_m[3].element(row, value[3]);
}

inline Vector4 Matrix4::row(unsigned int row) const
{
	assert(row < 4);

	return Vector4(_m[0].element(row), _m[1].element(row), _m[2].element(row), _m[3].element(row));
}

inline void Matrix4::Transform(Vector4& inout) const
{
	inout = Vector4(row(0).dot(inout),
				   row(1).dot(inout),
				   row(2).dot(inout),
				   row(3).dot(inout));
}

inline void Matrix4::Identity(Matrix4& out)
{
	out = Matrix4(Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0),
				  Vector4(0, 0, 1, 0), Vector4(0, 0, 0, 1));
}

inline void Matrix4::Translation(Matrix4& out, const Vector3& translation)
{
	out = Matrix4(Vector4(1, 0, 0, 0), Vector4(0, 1, 0, 0),
				  Vector4(0, 0, 1, 0), Vector4(translation, 1));
}

inline void Matrix4::PerspectiveFov(Matrix4& out, float fov, float aspect, float nearclip, float farclip)
{
	float f = 1 / tan( fov * ( PI / 180 ) / 2 );
	out = Matrix4(Vector4(f / aspect, 0, 0, 0),
				  Vector4(0, f, 0, 0),
				  Vector4(0, 0, (farclip + nearclip) / (nearclip - farclip), -1),
				  Vector4(0, 0, (2 * farclip * nearclip) / (nearclip - farclip), 0));
}

inline void Matrix4::LookAt(Matrix4& out, const Vector3& eye, const Vector3& at, const Vector3& up)
{
	Vector3 upnorm = up.unit();
	Vector3 forward = (at - eye).normalize();
	Vector3 side = forward.cross(upnorm);
	upnorm = side.cross(forward);

	Matrix4 translate;
	Translation(translate, -eye);

	out = Matrix4(Vector4(side.x(), upnorm.x(), -forward.x(), 0),
					Vector4(side.y(), upnorm.y(), -forward.y(), 0),
					Vector4(side.z(), upnorm.z(), -forward.z(), 0),
					Vector4(0, 0, 0, 1)) * translate;
}

inline void Matrix4::Scale(Matrix4& out, float scale)
{
	out = Matrix4(Vector4(scale, 0, 0, 0),
				Vector4(0, scale, 0, 0),
				Vector4(0, 0, scale, 0),
				Vector4(0, 0, 0, 1));
}

inline void Matrix4::Scale(Matrix4& out, const Vector3& scale)
{
	out = Matrix4(Vector4(scale.x(), 0, 0, 0),
				Vector4(0, scale.y(), 0, 0),
				Vector4(0, 0, scale.z(), 0),
				Vector4(0, 0, 0, 1));
}

inline void Matrix4::RotationAxis(Matrix4& out, const Vector3& axis, float angle)
{
	float sinTheta = sin(angle);
	float cosTheta = cos(angle);
	float oneMinusCosTheta = 1 - cosTheta;
	float axisXSquared = axis.x() * axis.x();
	float axisYSquared = axis.y() * axis.y();
	float axisZSquared = axis.z() * axis.z();

	out = Matrix4(Vector4(axisXSquared + (1-axisXSquared) * cosTheta,
		axis.x() * axis.y() * oneMinusCosTheta - axis.z() * sinTheta,
		axis.x() * axis.z() * oneMinusCosTheta + axis.y() * sinTheta,
		0),
		Vector4(axis.x() * axis.y() * oneMinusCosTheta + axis.z() * sinTheta,
		axisYSquared + (1 - axisYSquared) * cosTheta,
		axis.y() * axis.z() * oneMinusCosTheta - axis.x() * sinTheta,
		0),
		Vector4(axis.x() * axis.y() * oneMinusCosTheta - axis.y() * sinTheta,
		axis.y() * axis.z() * oneMinusCosTheta + axis.x() * sinTheta,
		axisZSquared + (1 - axisZSquared) * cosTheta,
		0),
		Vector4(0, 0, 0, 1));
}

inline Matrix4 Matrix4::operator* (const Matrix4& rhs) const
{
	return Matrix4(*this) *= rhs;
}

inline Vector4 operator* (const Matrix4& lhs, const Vector4& rhs)
{
	Vector4 v = rhs;
	lhs.Transform(v);
	return v;
}

inline std::ostream& operator<<(std::ostream& lhs, const Matrix4& rhs)
{
	lhs << "m4(" << rhs.column(0) << ", " << rhs.column(1) << ", ";
	lhs << rhs.column(2) << ", " << rhs.column(3) << ")";
	return lhs;
}

inline std::istream& operator>>(std::istream& lhs, Matrix4& rhs)
{
	Vector4 column[4];
	char c;
	lhs >> c >> c >> c >> column[0] >> c >> column[1] >> c >> column[2] >> c >> column[3] >> c;
	rhs = Matrix4(column[0], column[1], column[2], column[3]);
	return lhs;
}