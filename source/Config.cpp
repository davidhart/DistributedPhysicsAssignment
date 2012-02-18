// David Hart - 2011

#include "Config.h"
#include "Util.h"
#include "Application.h"
#include <fstream>
#include <string>

Config::Config()
{

}

bool Config::Read(const char* filename, Application& application)
{
	std::ifstream file(filename);

	if (!file.good())
		return false;

	return true;
}