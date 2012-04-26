#include "NetworkController.h"
#include "World.h"
#include "PhysicsThreads.h"
#include <iostream>

using namespace Networking;

const std::string NetworkController::BROADCAST_STRING("Is anyone there?");
const std::string NetworkController::BROADCAST_REPLY_STRING("Yes I am here");

NetworkController::~NetworkController()
{

}

SessionMasterController::SessionMasterController(GameWorldThread& worldThread) :
	_tcpListenSocket(TCPLISTEN_PORT),
	_worldThread(worldThread)
{
	_broadcastListenSocket.Bind(BROADCAST_PORT);
	_broadcastListenSocket.SetBlocking(false);

	_tcpListenSocket.SetBlocking(false);

	_broadcastReplyMessage.Append(BROADCAST_REPLY_STRING);
	_broadcastReplyMessage.Append(TCPLISTEN_PORT);
}

SessionMasterController::~SessionMasterController()
{
}

void SessionMasterController::DoTick()
{
	Message message;

	// If we have a client
	if (_clientSocket.IsOpen())
	{
		if (!_clientSocket.IsOpen())
		{
			std::cout << "Client timed out" << std::endl;
			_clientSocket.Close();
		}
	}
	else
	{
		Address address;

		// Reply to broadcasts
		while(_broadcastListenSocket.RecieveFrom(address, message))
		{
			std::string broadcastString;
			if (message.Read(broadcastString) && broadcastString == BROADCAST_STRING)
			{
				_broadcastListenSocket.SendTo(address, _broadcastReplyMessage);
			}
		}

		// Accept an incoming connection request
		if (_tcpListenSocket.Accept(_clientSocket, address))
		{
			std::cout << "Client Connected " << address << std::endl;
			_clientSocket.SetBlocking(true);
			_clientSocket.SetTimeout(5000);

			SendSessionInitialization();
		}
	}
}

void SessionMasterController::SendSessionInitialization()
{
	Message message;

	int numObjects = _worldThread._world->GetNumObjects();
	message.Append(numObjects);

	int objectStride = sizeof(float) * 5 + sizeof(unsigned int) * 2;

	for (int i = 0; i < numObjects; ++i)
	{
		if (message.Size() + objectStride > Message::MAX_BUFFER_SIZE)
		{
			if (!_clientSocket.Send(message))
			{
				std::cout << "???" << std::endl;
			}
			std::cout << message.Size() << " " << i << std::endl;
			message.Clear();
		}
		
		Physics::PhysicsObject* object = _worldThread._world->GetObject(i);

		message.Append(object->GetSerializationType());
		message.Append((float)object->GetPosition().x());
		message.Append((float)object->GetPosition().y());
		message.Append((float)object->GetVelocity().x());
		message.Append((float)object->GetVelocity().y());
		message.Append(object->GetColor().To32BitColor());
		message.Append((float)object->GetMass());
	}

	if (message.Size() != sizeof(unsigned short))
	{
		_clientSocket.Send(message);

		std::cout << message.Size() << std::endl;
	}
}

WorkerController::WorkerController(GameWorldThread& worldThread) :
	_worldThread(worldThread)
{
	_broadcastSocket.SetBlocking(false);

	_broadcastMessage.Append(BROADCAST_STRING);
}

WorkerController::~WorkerController()
{

}

void WorkerController::DoTick()
{
	// If we are connected to the session master
	if (_serverSocket.IsOpen())
	{
		DoConnectedTick();
	}
	else
	{
		DoFindHostTick();
	}
}

void WorkerController::DoConnectedTick()
{
	if (!_serverSocket.IsOpen())
	{
		std::cout << "Server timed out" << std::endl;
		_serverSocket.Close();
		// Revert to standalone mode??
	}
}

void WorkerController::DoFindHostTick()
{
	unsigned short port;

	_broadcastSocket.Broadcast(_broadcastMessage, BROADCAST_PORT);

	Address serverAddress;
	Message message;

	// If we don't have a reply
	if (!_broadcastSocket.RecieveFrom(serverAddress, message))
	{
		return;
	}
	else if (ValidateBroadcastResponseAndGetPort(message, port))
	{
		serverAddress.SetPort(port);

		// Attempt to connect
		std::cout << "Attempting to connect to server " << serverAddress << std::endl;
		_serverSocket = TcpSocket(serverAddress);

		_serverSocket.SetTimeout(5000);

		if (_serverSocket.IsOpen())
		{
			std::cout << "Connected, awaiting initialization data" << std::endl;

			ReceiveInitialisationData();
		}
		else
		{
			std::cout << "Connection failed" << std::endl;
		}
	}
}

void WorkerController::ReceiveInitialisationData()
{
	_worldThread._world->ClearObjects();

	Message message;
	if (!_serverSocket.Receive(message))
	{
		return;
	}

	int numObjects = 0;
	int numObjectsRead = 0;

	if (!message.Read(numObjects))
	{
		_serverSocket.Close();
	}
	
	std::cout << "Recieving " << numObjects << " objects" << std::endl;

	_worldThread._world->ClearObjects();

	while(numObjectsRead < numObjects && _serverSocket.IsOpen())
	{						
		unsigned objectType = 0;

		// If reading an object failed try to read a new packet
		if (!message.Read(objectType))
		{
			// If reading a packet failed (timeout) then the connection
			// could not complete so exit and the socket should have closed
			if (!_serverSocket.Receive(message))
				return;

			std::cout << message.Size() << std::endl;
		}
		// Reading object type was successul so continue to read remainder of object
		else
		{
			float x, y; // Position
			float vx, vy; // Velocity
			unsigned int color; // 32bit ABGR color
			float mass;

			bool valid = true;

			valid &= message.Read(x);
			valid &= message.Read(y);
			valid &= message.Read(vx);
			valid &= message.Read(vy);
			valid &= message.Read(color);
			valid &= message.Read(mass);

			Physics::PhysicsObject* object = NULL;
			if (objectType == 0)
			{
				object = _worldThread._world->AddBox();
			}
			else
			{
				// TODO: other object types
				std::cout << "???" << std::endl;
			}

			object->SetPosition(Vector2d((double)x, (double)y));
			object->SetVelocity(Vector2d((double)x, (double)y));
			object->SetMass((double)mass);
			object->SetColor(Color(color));

			numObjectsRead++;
		}
	}
}

bool WorkerController::ValidateBroadcastResponseAndGetPort(Networking::Message& message, unsigned short& port)
{
	std::string replyString;

	// Verify the reply contains a string in the correct format
	if (message.Read(replyString) && replyString == BROADCAST_REPLY_STRING)
	{
		// Vertify the reply contains a port
		if (message.Read(port))
		{
			return true;
		}
	}
	
	return false;
}