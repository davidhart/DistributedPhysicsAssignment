// David Hart - 2011

#include "Util.h"
#include "VertexBuffer.h"

#include <fstream>
#include <cassert>

void Util::ReadTextFileToString(const std::string& filename, std::string& text)
{
	std::ifstream file(filename);
	assert(file.is_open());

	text.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

bool Util::FloatEquality(double a, double b, double precision)
{
	double difference = a - b;
	return difference < precision && difference > -precision;
}

bool Util::FloatEquality(float a, float b, float precision)
{
	float difference = a - b;
	return difference < precision && difference > -precision;
}

bool Util::ReadToken(std::istream& stream, const char* token)
{
	std::string temp;
	stream >> temp;
	return token == temp;
}