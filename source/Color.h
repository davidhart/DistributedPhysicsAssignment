// David Hart - 2012

#pragma once

#include "Vector.h"

class Color
{

public:

	explicit Color(unsigned int color); // from 32bit color
	Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
	Color(float r, float g, float b, float a = 1.0f);
	Color();

	Vector3f ToVector3f();
	Vector4f ToVector4f();
	unsigned int To32BitColor();

	void SetRed(unsigned char red);
	void SetGreen(unsigned char green);
	void SetBlue(unsigned char blue);
	void SetAlpha(unsigned char alpha);

	void SetRed(float red);
	void SetGreen(float green);
	void SetBlue(float blue);
	void SetAlpha(float alpha);

	unsigned char GetRed();
	unsigned char GetGreen();
	unsigned char GetBlue();
	unsigned char GetAlpha();

private:

	unsigned int _color;
};