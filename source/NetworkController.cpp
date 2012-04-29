#include "NetworkController.h"
#include "World.h"
#include "PhysicsThreads.h"
#include <iostream>

using namespace Networking;

const std::string NetworkController::BROADCAST_STRING("Is anyone there?");
const std::string NetworkController::BROADCAST_REPLY_STRING("Yes I am here");

NetworkController::NetworkController() :
	_shutDown(false)
{

}

NetworkController::~NetworkController()
{
	Shutdown();
	Join();
}

unsigned NetworkController::ThreadMain()
{
	while(!_shutDown)
	{
		// TODO: implement timer to control tickrate & timeouts
		DoTick();

		Sleep(1);
	}

	return 0;
}

void NetworkController::Shutdown()
{
	_shutDown = true;
}

SessionMasterController::SessionMasterController(GameWorldThread& worldThread) :
	_tcpListenSocket(TCPLISTEN_PORT),
	_worldThread(worldThread),
	_state(LISTENING_CLIENT)
{
	_broadcastListenSocket.Bind(BROADCAST_PORT);
	_broadcastListenSocket.SetBlocking(false);

	_tcpListenSocket.SetBlocking(false);

	_broadcastReplyMessage.Append(BROADCAST_REPLY_STRING);
	_broadcastReplyMessage.Append(TCPLISTEN_PORT);
}

void SessionMasterController::ExchangeState()
{
	Threading::ScopedLock lock(_exchangeMutex);

	// Make a copy of the world state to send to the client
	if (_state == WAIT_ON_INITIALISATION_GATHER)
	{
		unsigned numObjects = _worldThread._world->GetNumObjects();
		_initialisationData._messagesSent = 0;
		const int objectStride = sizeof(unsigned) * 2 + sizeof(double) * 5;

		Message message;
		message.Append(numObjects);

		for (unsigned i = 0; i < numObjects; ++i)
		{
			// If another object can't fit into the message, add it onto the list
			// and start building another message
			if (message.Size() + objectStride > Message::MAX_BUFFER_SIZE)
			{
				_initialisationData._messages.push_back(message);
				message.Clear();
			}

			Physics::PhysicsObject* object = _worldThread._world->GetObject(i);

			message.Append(object->GetSerializationType());
			message.Append(object->GetPosition().x());
			message.Append(object->GetPosition().y());
			message.Append(object->GetVelocity().x());
			message.Append(object->GetVelocity().y());
			message.Append(object->GetColor().To32BitColor());
			message.Append(object->GetMass());
		}

		if (message.Size() != sizeof(unsigned short))
		{
			_initialisationData._messages.push_back(message);
		}

		_sendData._messagesSent = 0;

		_state = ACCEPTING_CLIENT;
	}
	// For now synchronise all objects
	else if (_state == SYNCHRONISE_CLIENT)
	{
		_sendData._newState.clear();

		unsigned numObjects = _worldThread._world->GetNumObjects();
		const int objectStride = sizeof(unsigned) * 1 + sizeof(double) * 4;

		Message message;
		message.Append(numObjects);

		for (unsigned i = 0; i < numObjects; ++i)
		{
			// If another object can't fit into the message, add it onto the list
			// and start building another message
			if (message.Size() + objectStride > Message::MAX_BUFFER_SIZE)
			{
				_sendData._newState.push_back(message);
				message.Clear();
			}

			Physics::PhysicsObject* object = _worldThread._world->GetObject(i);

			message.Append(i);
			message.Append(object->GetPosition().x());
			message.Append(object->GetPosition().y());
			message.Append(object->GetVelocity().x());
			message.Append(object->GetVelocity().y());
		}

		if (message.Size() != sizeof(unsigned short))
		{
			_sendData._newState.push_back(message);
		}
	}
}

void SessionMasterController::DoTick()
{
	// If something caused the client to disconnect, listen for a new client
	if (_state != LISTENING_CLIENT && !_clientSocket.IsOpen())
	{
		std::cout << "revert to listen state" << std::endl;
		_state = LISTENING_CLIENT;
	}

	switch(_state)
	{
	case LISTENING_CLIENT:
		DoAcceptHostTick();
		break;

	case ACCEPTING_CLIENT:
		SendSessionInitialization();
		break;

	case SYNCHRONISE_CLIENT:
		DoPeerConnectedTick();
		break;
	}

	UpdatePeerId();
}

void SessionMasterController::DoAcceptHostTick()
{
	Message message;
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
		Threading::ScopedLock lock(_exchangeMutex);
		std::cout << "Client Connected " << address << std::endl;
		_clientSocket.SetBlocking(false);
		_state = WAIT_ON_INITIALISATION_GATHER;
	}

}

void SessionMasterController::SendSessionInitialization()
{
	while (_initialisationData._messagesSent < _initialisationData._messages.size())
	{
		// If sending would block, give up for now
		if (!_clientSocket.Send(_initialisationData._messages[_initialisationData._messagesSent]))
		{
			return;
		}
		
		 _initialisationData._messagesSent++;
	}

	// If initialisation complete
	if (_initialisationData._messagesSent == _initialisationData._messages.size())
	{
		_initialisationData._messages.clear();
		_state = SYNCHRONISE_CLIENT;
	}
}


void SessionMasterController::DoPeerConnectedTick()
{
	while (_sendData._messagesSent < _sendData._sendingState.size())
	{
		// If sending would block, give up for now
		if (!_clientSocket.Send(_sendData._sendingState[_sendData._messagesSent]))
		{
			return;
		}
		
		 _sendData._messagesSent++;
	}

	// If initialisation complete
	if (_sendData._messagesSent == _sendData._sendingState.size())
	{
		_sendData._messagesSent = 0;

		Threading::ScopedLock lock(_exchangeMutex);
		
		// Replace the most recent state with the one we just finished sending
		_sendData._newState.swap(_sendData._sendingState);
	}

	if (!_clientSocket.IsOpen())
	{
		std::cout << "Client timed out" << std::endl;
		_clientSocket.Close();
	}
}

void SessionMasterController::UpdatePeerId()
{
	if (_clientSocket.IsOpen())
	{
		_worldThread.SetPeerId(0);
		_worldThread.SetNumPeers(2);
	}
	else
	{
		_worldThread.SetPeerId(0);
		_worldThread.SetNumPeers(1);
	}
}

WorkerController::WorkerController(GameWorldThread& worldThread) :
	_worldThread(worldThread),
	_state(FINDING_HOST)
{
	_broadcastSocket.SetBlocking(false);
	_broadcastMessage.Append(BROADCAST_STRING);
}

void WorkerController::DoTick()
{
	// If something caused the socket to close, find a new host
	if (_state != FINDING_HOST && !_serverSocket.IsOpen())
	{
		std::cout << "revert to find state" << std::endl;
		_state = FINDING_HOST;
	}

	switch(_state)
	{
	case FINDING_HOST:
		DoFindHostTick();
		break;

	case RECEIVING_INITIALISATION:
		ReceiveInitialisationData();
		break;

	case SYNCHRONISE_CLIENT:
		DoConnectedTick();
		break;

	}

	UpdatePeerId();
}

void WorkerController::UpdatePeerId()
{
	if (_state == SYNCHRONISE_CLIENT)
	{
		_worldThread.SetPeerId(1);
		_worldThread.SetNumPeers(2);
	}
	else
	{
		_worldThread.SetPeerId(0);
		_worldThread.SetNumPeers(1);
	}
}

void WorkerController::DoConnectedTick()
{
	// Store parts of frames as they arrive
	Message message;

	while (_serverSocket.Receive(message))
	{
		if (_updateData._objectsReceived.size() == 0)
		{
			unsigned numObjects = 0;
			if (!message.Read(numObjects))
			{
				CloseConnection();
				return;
			}

			_updateData._objectsReceived.resize(numObjects);
			_updateData._objectsRead = 0;
		}


		while(_updateData._objectsRead < _updateData._objectsReceived.size())
		{
			ObjectState& object = _updateData._objectsReceived[_updateData._objectsRead];

			// If message doesn't contain another id wait for next message
			if (!message.Read(object.id))
			{
				break;
			}

			bool valid = true;
			valid &= message.Read(object.x);
			valid &= message.Read(object.y);
			valid &= message.Read(object.vx);
			valid &= message.Read(object.vy);

			// If malformed object
			if (!valid)
			{
				CloseConnection();
				return;
			}

			_updateData._objectsRead++;
		}

		if (_updateData._objectsRead == _updateData._objectsReceived.size())
		{
			Threading::ScopedLock lock(_exchangeMutex);

			_updateData._objectsReceived.swap(_updateData._objectsUpdate);
			_updateData._objectsReceived.clear();
			_updateData._objectsRead = 0;
		}
	}

	if (!_serverSocket.IsOpen())
	{
		std::cout << "Server timed out" << std::endl;
		_serverSocket.Close();
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
		
		if (_serverSocket.IsOpen())
		{
			Threading::ScopedLock lock(_exchangeMutex);
			std::cout << "Connected, awaiting initialization data" << std::endl;
			_serverSocket.SetBlocking(false);
			_initialisationData._objects.clear();
			_state = RECEIVING_INITIALISATION;
		}
		else
		{
			std::cout << "Connection failed" << std::endl;
		}
	}
}

void WorkerController::ReceiveInitialisationData()
{
	Message message;
	while (_serverSocket.Receive(message) && _serverSocket.IsOpen())
	{
		if (_initialisationData._objects.size() == 0)
		{
			int objectsToRead;
			if (!message.Read(objectsToRead))
			{
				CloseConnection();
				return;
			}
			std::cout << "Recieving " << objectsToRead << " objects" << std::endl;
			_initialisationData._objectsRead = 0;
			_initialisationData._objects.resize(objectsToRead);
		}

		while(_initialisationData._objectsRead < _initialisationData._objects.size() 
					&& _serverSocket.IsOpen())
		{						
			unsigned objectType = 0;

			// If reading an object failed exit and wait for next message
			if (!message.Read(objectType))
			{
				return;
			}
			// Reading object type was successul so continue to read remainder of object
			else
			{
				bool valid = true;

				ObjectInitialisation& objectInit = _initialisationData._objects[_initialisationData._objectsRead];

				valid &= message.Read(objectInit.x);
				valid &= message.Read(objectInit.y);
				valid &= message.Read(objectInit.vx);
				valid &= message.Read(objectInit.vy);
				valid &= message.Read(objectInit.color);
				valid &= message.Read(objectInit.mass);

				if (!valid)
				{
					CloseConnection();
					return;
				}

				_initialisationData._objectsRead++;
			}
		}

		// If initialisation complete
		if (_initialisationData._objectsRead == _initialisationData._objects.size())
		{
			Threading::ScopedLock lock(_exchangeMutex);
			_state = RECEIVED_INITIALISATION;		
			return;
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

void WorkerController::CloseConnection()
{
	_serverSocket.Close();
	_state = FINDING_HOST;
}

void WorkerController::ExchangeState()
{
	Threading::ScopedLock lock(_exchangeMutex);

	if (_state == RECEIVED_INITIALISATION)
	{
		std::cout << "received " << _initialisationData._objects.size() << "objects " << std::endl;
		_worldThread._world->ClearObjects();

		for (unsigned i = 0; i < _initialisationData._objects.size(); ++i)
		{
			Physics::PhysicsObject* object = NULL;
			const ObjectInitialisation& objectInit = _initialisationData._objects[i];

			if (objectInit.objectType == 0)
			{
				object = _worldThread._world->AddBox();
			}

			object->SetPosition(Vector2d(objectInit.x, objectInit.y));
			object->SetVelocity(Vector2d(objectInit.vx, objectInit.vy));
			object->SetMass(objectInit.mass);
			object->SetColor(Color(objectInit.color));
			object->UpdateShape(*_worldThread._world);
		}

		_initialisationData._objects.clear();
		_updateData._objectsRead = 0;
		_state = SYNCHRONISE_CLIENT;
	}
	else if (_state == SYNCHRONISE_CLIENT)
	{
		for (unsigned i = 0; i < _updateData._objectsUpdate.size(); ++i)
		{
			const ObjectState& objectState = _updateData._objectsUpdate[i];
			Physics::PhysicsObject* object = _worldThread._world->GetObject(objectState.id);

			object->SetPosition(Vector2d(objectState.x, objectState.y));
			object->SetVelocity(Vector2d(objectState.vx, objectState.vy));
		}

		// Clear these updates to be sure we don't apply them again
		_updateData._objectsUpdate.clear();
	}
}
