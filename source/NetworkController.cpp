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

void SessionMasterController::DoTickComplete()
{
	Message message;

	// If we have a client
	if (_clientSocket.IsOpen())
	{
		// Sync objects positions after collision resolution
		SyncCollisionObjects();

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

	UpdatePeerId();
}

void SessionMasterController::BeginTick()
{
	// If we have a client
	if (_clientSocket.IsOpen())
	{
		Message message;
		message.Append(_worldThread.GetStepDelta());

		// TODO: handle adding / removing objects?

		_clientSocket.Send(message);
	}

	UpdatePeerId();
}

void SessionMasterController::PrepareForCollisions()
{
	if (_clientSocket.IsOpen())
	{
		// Sync objects integrated positions with peer
		SyncIntegratedObjects();
	}

	UpdatePeerId();
}

void SessionMasterController::SyncIntegratedObjects()
{
	// Send objects state to peer
	Message message;

	int objectStride = sizeof(double)*4;
	int serverStartIndex =  PhysicsWorkerThread::GetStartIndexForId(0, 2, _worldThread._world->GetNumObjects());
	int serverEndIndex = PhysicsWorkerThread::GetEndIndexForId(0, 2, _worldThread._world->GetNumObjects());

	for (int i = serverStartIndex; i <= serverEndIndex; ++i)
	{
		if (message.Size() + objectStride > Message::MAX_BUFFER_SIZE)
		{
			_clientSocket.Send(message);
			message.Clear();
		}

		Physics::PhysicsObject* object = _worldThread._world->GetObject(i);

		message.Append(object->GetPosition().x());
		message.Append(object->GetPosition().y());
		message.Append(object->GetVelocity().x());
		message.Append(object->GetVelocity().y());
	}

	if (message.Size() != sizeof(unsigned short))
	{
		_clientSocket.Send(message);
	}

	// Recieve objects state from peer
	int clientStartIndex = PhysicsWorkerThread::GetStartIndexForId(1, 2, _worldThread._world->GetNumObjects());
	int clientEndIndex =  PhysicsWorkerThread::GetEndIndexForId(1, 2, _worldThread._world->GetNumObjects());
	message.Clear();
	while(clientStartIndex <= clientEndIndex && _clientSocket.IsOpen())
	{						
		double x, y; // Position
		double vx, vy; // Velocity

		// If reading an object failed try to read a new packet
		if (!message.Read(x))
		{
			// If reading a packet failed (timeout) then the connection
			// could not complete so exit and the socket should have closed
			_clientSocket.Receive(message);
		}
		// Reading object type was successul so continue to read remainder of object
		else
		{
			bool valid = true;

			valid &= message.Read(y);
			valid &= message.Read(vx);
			valid &= message.Read(vy);

			if(!valid)
			{
				_clientSocket.Close();
				break;
			}

			Physics::PhysicsObject* object = _worldThread._world->GetObject(clientStartIndex);

			object->SetPosition(Vector2d(x, y));
			object->SetVelocity(Vector2d(vx, vy));

			clientStartIndex++;
		}
	}
}

void SessionMasterController::SyncCollisionObjects()
{
	const unsigned objectStride = sizeof(unsigned) + sizeof(double) * 4;

	// Calculate the number of objects in buckets this peer is responsible for
	// TODO: extract into method or cache somewhere
	unsigned numObjectsToSend = 0;

	int xMin = PhysicsWorkerThread::GetStartIndexForId(0, 2, _worldThread._world->GetNumBucketsWide());
	int xMax = PhysicsWorkerThread::GetEndIndexForId(0, 2, _worldThread._world->GetNumBucketsWide());
	int yMax = _worldThread._world->GetNumBucketsTall();

	for (int x = xMin; x <= xMax; ++x)
	{
		for (int y = 0; y < yMax; ++y)
		{
			numObjectsToSend += _worldThread._world->GetNumObjectsInBucket(x, y);
		}
	}

	Message message;

	message.Append(numObjectsToSend);
	
	// Send objects this peer is responsible for
	for (int x = xMin; x <= xMax; ++x)
	{
		for (int y = 0; y < yMax; ++y)
		{
			const std::vector<unsigned>& bucket = _worldThread._world->GetObjectsInBucket(x, y);

			for (unsigned i = 0; i < bucket.size(); ++i)
			{
				if (message.Size() + objectStride > Message::MAX_BUFFER_SIZE)
				{
					_clientSocket.Send(message);
					message.Clear();
				}

				Physics::PhysicsObject* object = _worldThread._world->GetObject(bucket[i]);

				message.Append(bucket[i]);
				message.Append(object->GetPosition().x());
				message.Append(object->GetPosition().y());
				message.Append(object->GetVelocity().x());
				message.Append(object->GetVelocity().y());
			}
		}
	}

	if (message.Size() != sizeof(unsigned short))
	{
		_clientSocket.Send(message);
	}

	message.Clear();

	// Receive and update objects that collided on the server
	_clientSocket.Receive(message);

	unsigned numObjectsToReceive;

	if (!message.Read(numObjectsToReceive))
	{
		_clientSocket.Close();
		return;
	}

	// Sanity check
	assert((int)numObjectsToReceive <= _worldThread._world->GetNumObjects());

	unsigned received = 0;

	while (_clientSocket.IsOpen() && received < numObjectsToReceive)
	{
		unsigned objectId;
		if (!message.Read(objectId))
		{
			_clientSocket.Receive(message);
		}
		else
		{
			double x, y, vx, vy;
			
			bool valid = true;

			valid &= message.Read(x);
			valid &= message.Read(y);
			valid &= message.Read(vx);
			valid &= message.Read(vy);

			if (!valid)
			{
				_clientSocket.Close();
				return;
			}
			else
			{
				Physics::PhysicsObject* object = _worldThread._world->GetObject(objectId);

				object->SetPosition(Vector2d(x, y));
				object->SetVelocity(Vector2d(vx, vy));

				object->UpdateShape(*_worldThread._world);
				received++;
			}
		}
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

void SessionMasterController::SendSessionInitialization()
{
	Message message;

	int numObjects = _worldThread._world->GetNumObjects();
	message.Append(numObjects);

	int objectStride = sizeof(double) * 5 + sizeof(unsigned int) * 2;

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
		message.Append(object->GetPosition().x());
		message.Append(object->GetPosition().y());
		message.Append(object->GetVelocity().x());
		message.Append(object->GetVelocity().y());
		message.Append(object->GetColor().To32BitColor());
		message.Append(object->GetMass());
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

void WorkerController::DoTickComplete()
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

	UpdatePeerId();
}

void WorkerController::BeginTick()
{
	// If we are connected to the session master
	if (_serverSocket.IsOpen())
	{
		Message message;
		
		_serverSocket.Receive(message);

		double delta;
		if (!message.Read(delta))
		{
			_serverSocket.Close();
		}

		_worldThread.SetStepDelta(delta);
	}

	UpdatePeerId();
}

void WorkerController::PrepareForCollisions()
{
	// If we are connected to the session master
	if (_serverSocket.IsOpen())
	{
		SyncIntegratedObjects();
	}

	UpdatePeerId();
}

void WorkerController::SyncIntegratedObjects()
{
	Message message;

	// Recieve objects state from peer
	int serverStartIndex = PhysicsWorkerThread::GetStartIndexForId(0, 2, _worldThread._world->GetNumObjects());
	int serverEndIndex = PhysicsWorkerThread::GetEndIndexForId(0, 2, _worldThread._world->GetNumObjects());

	while(serverStartIndex <= serverEndIndex && _serverSocket.IsOpen())
	{						
		double x, y; // Position
		double vx, vy; // Velocity

		// If reading an object failed try to read a new packet
		if (!message.Read(x))
		{
			// If reading a packet failed (timeout) then the connection
			// could not complete so exit and the socket should have closed
			_serverSocket.Receive(message);
		}
		// Reading object type was successul so continue to read remainder of object
		else
		{
			bool valid = true;

			valid &= message.Read(y);
			valid &= message.Read(vx);
			valid &= message.Read(vy);

			if(!valid)
			{
				_serverSocket.Close();
				break;
			}

			Physics::PhysicsObject* object = _worldThread._world->GetObject(serverStartIndex);

			object->SetPosition(Vector2d(x, y));
			object->SetVelocity(Vector2d(vx, vy));

			serverStartIndex++;
		}
	}

	// Send objects state to peer
	int objectStride = sizeof(double)*4;
	int clientStartIndex =  PhysicsWorkerThread::GetStartIndexForId(1, 2, _worldThread._world->GetNumObjects());
	int clientEndIndex = PhysicsWorkerThread::GetEndIndexForId(1, 2, _worldThread._world->GetNumObjects());
	message.Clear();

	for (int i = clientStartIndex; i <= clientEndIndex; ++i)
	{
		if (message.Size() + objectStride > Message::MAX_BUFFER_SIZE)
		{
			_serverSocket.Send(message);
			message.Clear();
		}

		Physics::PhysicsObject* object = _worldThread._world->GetObject(i);

		message.Append(object->GetPosition().x());
		message.Append(object->GetPosition().y());
		message.Append(object->GetVelocity().x());
		message.Append(object->GetVelocity().y());
	}

	if (message.Size() != sizeof(unsigned short))
	{
		_serverSocket.Send(message);
	}
}

void WorkerController::SyncCollisionObjects()
{
	const unsigned objectStride = sizeof(unsigned) + sizeof(double) * 4;

	// Calculate the number of objects in buckets this peer is responsible for
	// TODO: extract into method or cache somewhere
	unsigned numObjectsToSend = 0;

	int xMin = PhysicsWorkerThread::GetStartIndexForId(1, 2, _worldThread._world->GetNumBucketsWide());
	int xMax = PhysicsWorkerThread::GetEndIndexForId(1, 2, _worldThread._world->GetNumBucketsWide());
	int yMax = _worldThread._world->GetNumBucketsTall();

	for (int x = xMin; x <= xMax; ++x)
	{
		for (int y = 0; y < yMax; ++y)
		{
			numObjectsToSend += _worldThread._world->GetNumObjectsInBucket(x, y);
		}
	}

	Message message;

	_serverSocket.Receive(message);

	unsigned numObjectsToReceive;

	if (!message.Read(numObjectsToReceive))
	{
		_serverSocket.Close();
		return;
	}

	// Sanity check
	assert((int)numObjectsToReceive <= _worldThread._world->GetNumObjects());

	unsigned received = 0;

	// Receive and update objects that collided on the server
	while (_serverSocket.IsOpen() && received < numObjectsToReceive)
	{
		unsigned objectId;
		if (!message.Read(objectId))
		{
			_serverSocket.Receive(message);
		}
		else
		{
			double x, y, vx, vy;
			
			bool valid = true;

			valid &= message.Read(x);
			valid &= message.Read(y);
			valid &= message.Read(vx);
			valid &= message.Read(vy);

			if (!valid)
			{
				_serverSocket.Close();
				return;
			}
			else
			{
				Physics::PhysicsObject* object = _worldThread._world->GetObject(objectId);

				object->SetPosition(Vector2d(x, y));
				object->SetVelocity(Vector2d(vx, vy));

				object->UpdateShape(*_worldThread._world);
				received++;
			}
		}
	}

	message.Clear();

	message.Append(numObjectsToSend);

	// Send objects this peer is responsible for
	for (int x = xMin; x <= xMax; ++x)
	{
		for (int y = 0; y < yMax; ++y)
		{
			const std::vector<unsigned>& bucket = _worldThread._world->GetObjectsInBucket(x, y);

			for (unsigned i = 0; i < bucket.size(); ++i)
			{
				if (message.Size() + objectStride > Message::MAX_BUFFER_SIZE)
				{
					_serverSocket.Send(message);
					message.Clear();
				}

				Physics::PhysicsObject* object = _worldThread._world->GetObject(bucket[i]);

				message.Append(bucket[i]);
				message.Append(object->GetPosition().x());
				message.Append(object->GetPosition().y());
				message.Append(object->GetVelocity().x());
				message.Append(object->GetVelocity().y());
			}
		}
	}

	if (message.Size() != sizeof(unsigned short))
	{
		_serverSocket.Send(message);
	}
}

void WorkerController::UpdatePeerId()
{
	if (_serverSocket.IsOpen())
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
	SyncCollisionObjects();

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
			double x, y; // Position
			double vx, vy; // Velocity
			unsigned int color; // 32bit ABGR color
			double mass;

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

			object->SetPosition(Vector2d(x, y));
			object->SetVelocity(Vector2d(x, y));
			object->SetMass(mass);
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