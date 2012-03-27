// David Hart - 2011

#pragma once

#include <string>
#include <iosfwd>
#include "Maths.h"

class ObjFile;
class VertexBuffer;
class Renderer;

namespace Util
{
	const float EPSILON = 0.00001f;

	void ReadTextFileToString(const std::string& filename, std::string& text);

	bool FloatEquality(double a, double b, double precision = (double)EPSILON);
	bool FloatEquality(float a, float b, float precision = EPSILON);

	bool ReadToken(std::istream& stream, const char* token);

	void CreateObjFileWithBuffers(const char* filename, 
								  const Renderer& renderer,
								  ObjFile& objFile, 
								  VertexBuffer& vertexBuffer, 
								  VertexBuffer& indexBuffer);

	template <typename T> T Max(T a, T b)
	{
		return a > b ? a : b;
	}

	template <typename T> T Min(T a, T b)
	{
		return a < b ? a : b;
	}

	inline float DegreesToRadians(float degrees)
	{
		return degrees * PI / 180.0f;
	}

	inline float RadiansToDegrees(float radians)
	{
		return radians * 180.0f / PI;
	}

	inline float ByteToFloatColorComponent(unsigned char channel)
	{
		return channel / 255.0f;
	}

	inline unsigned char FloatToByteColorComponent(float channel)
	{
		return (unsigned char)(channel * 255.0f);
	}

	inline double RandRange(double min, double max)
	{
		return rand()/(double)RAND_MAX * (max - min) + min;
	}
};