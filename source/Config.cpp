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

	while(file.good())
	{
		std::string token;

		file >> token;

		if (token == "gravity")
		{
			double gravity;

			file >> gravity;

			application.SetGravity(gravity);
		}
		else if (token == "friction")
		{
			double friction;

			file >> friction;

			application.SetFriction(friction);
		}
		else if (token == "elasticity")
		{
			double elasticity;

			file >> elasticity;

			application.SetElasticity(elasticity);
		}
		else
		{
			return false;
		}
	}

	return true;
}