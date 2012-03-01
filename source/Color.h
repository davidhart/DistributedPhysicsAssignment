#pragma once

class Vector3;
class Vector4;

class Color
{

public:

	Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);
	Color(float r, float g, float b, float a = 1.0f);
	Color();

	Vector3 ToVector3();
	Vector4 ToVector4();

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