// David Hart - 2012

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
		else if (token == "listen_port")
		{
			unsigned short port;

			file >> port;

			NetworkController::TCPLISTEN_PORT = port;
		}
		else if (token == "broadcast_port")
		{
			unsigned short port;

			file >> port;

			NetworkController::BROADCAST_PORT = port;
		}
		else
		{
			return false;
		}
	}

	return true;
}