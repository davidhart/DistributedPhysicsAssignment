#include "Color.h"

Color::Color(unsigned char red, unsigned char green, unsigned char blue, unsigned char alpha)
{
	_color = red | (green << 8) | (blue << 16) | (alpha << 24);
}

Color::Color(float red, float green, float blue, float alpha)
{
	SetRed(red);
	SetGreen(green);
	SetBlue(blue);
	SetAlpha(alpha);
}

Color::Color() :
	_color(0xFFFFFFFF)
{
}

Vector3f Color::ToVector3f()
{
	return Vector3f(Util::ByteToFloatColorComponent(GetRed()),
		Util::ByteToFloatColorComponent(GetGreen()),
		Util::ByteToFloatColorComponent(GetBlue()));
}

Vector4f Color::ToVector4f()
{
	return Vector4f(Util::ByteToFloatColorComponent(GetRed()),
		Util::ByteToFloatColorComponent(GetGreen()),
		Util::ByteToFloatColorComponent(GetBlue()),
		Util::ByteToFloatColorComponent(GetAlpha()));
}

void Color::SetRed(unsigned char red)
{
	_color &= 0xFFFFFF00;
	_color |= red;
}

void Color::SetGreen(unsigned char green)
{
	_color &= 0xFFFF00FF;
	_color |= green << 8;
}

void Color::SetBlue(unsigned char blue)
{
	_color &= 0xFF00FFFF;
	_color |= blue << 16;
}

void Color::SetAlpha(unsigned char alpha)
{
	_color &= 0x00FFFFFF;
	_color |= alpha << 24;
}

void Color::SetRed(float red)
{
	SetRed(Util::FloatToByteColorComponent(red));
}

void Color::SetGreen(float green)
{
	SetGreen(Util::FloatToByteColorComponent(green));
}

void Color::SetBlue(float blue)
{
	SetBlue(Util::FloatToByteColorComponent(blue));
}

void Color::SetAlpha(float alpha)
{
	SetAlpha(Util::FloatToByteColorComponent(alpha));
}

unsigned char Color::GetRed()
{
	return (unsigned char)((_color >> 24) & 0xFF);
}

unsigned char Color::GetGreen()
{
	return (unsigned char)((_color >> 16) & 0xFF);
}

unsigned char Color::GetBlue()
{
	return (unsigned char)((_color >> 8) & 0xFF);
}

unsigned char Color::GetAlpha()
{
	return (unsigned char)(_color & 0xFF);
}