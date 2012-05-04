#include "NetworkController.h"
#include "World.h"
#include "PhysicsThreads.h"
#include <iostream>

using namespace Networking;

const std::string NetworkController::BROADCAST_STRING("Is anyone there?");
const std::string NetworkController::BROADCAST_REPLY_STRING("Yes I am here");

ObjectExchange::ObjectExchange(GameWorldThread& worldThread, unsigned peerId) :
	_worldThread(worldThread),
	_world(*(worldThread._world)),
	_peerId(peerId)
{
	Reset();
}

void ObjectExchange::Reset()
{
	_objectMigrationIn.clear();
	_objectMigrationOut.clear();

	_sendData._messagesSent = 0;
	_sendData._newState.clear();
	_sendData._sendingState.clear();

	_updateData._objectsRead = 0;
	_updateData._objectsReceived.clear();
	_updateData._objectsUpdate.clear();

	_initialisationDataOut._messagesSent = 0;
	_initialisationDataOut._messages.clear();

	
	_initialisationDataIn._objectsRead = 0;
	_initialisationDataIn._objects.clear();
	_initialisationDataIn._initialisationReceived = false;
}

void ObjectExchange::ReceiveState(TcpSocket& socket)
{
	// Store parts of frames as they arrive
	Message message;

	while (socket.Receive(message))
	{
		eMessageType messageType;

		if (!message.Read(messageType))
		{
			std::cout << "Invalid message received" << std::endl;
			socket.Close();
			return;
		}

		if (messageType == OBJECT_UPDATES)
		{
			HandleUpdateMessage(socket, message);
		}
		else if (messageType == OBJECT_MIGRATION)
		{
			HandleMigrationMessage(socket, message);
		}
		else
		{
			socket.Close();
			std::cout << "Invalid message received" << std::endl;
			return;
		}
	}

	if (!socket.IsOpen())
	{
		std::cout << "peer timed out" << std::endl;
	}
}

void ObjectExchange::HandleUpdateMessage(TcpSocket& socket, Message& message)
{
	// If this is the first update message of a batch
	if (_updateData._objectsReceived.size() == 0)
	{
		unsigned numObjects = 0;
		if (!message.Read(numObjects))
		{
			socket.Close();
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
			socket.Close();
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

void ObjectExchange::HandleMigrationMessage(TcpSocket& socket, Message& message)
{
	Threading::ScopedLock lock (_exchangeMutex);

	ObjectMigration migration;

	while(message.Read(migration.type))
	{
		if (!message.Read(migration.objectId))
		{
			socket.Close();
			return;
		}
		
		_objectMigrationIn.push_back(migration);
	}
}

void ObjectExchange::SendState(TcpSocket& socket)
{
	// Try to send waiting messages messages
	while (_sendData._messagesSent < _sendData._sendingState.size())
	{

		// If sending would block, give up for now
		if (!socket.Send(_sendData._sendingState[_sendData._messagesSent]))
		{
			return;
		}
		
		 _sendData._messagesSent++;
	}

	// If complete frame sent
	if (_sendData._messagesSent == _sendData._sendingState.size())
	{
		_sendData._messagesSent = 0;

		Threading::ScopedLock lock(_exchangeMutex);
		
		// Replace the most recent state with the one we just finished sending
		_sendData._newState.swap(_sendData._sendingState);
	}

	{
		Threading::ScopedLock lock(_exchangeMutex);
		if (_objectMigrationOut.size() != 0)
		{
			Message message;
			message.Append(OBJECT_MIGRATION);
			
			const int objectStride = sizeof(eRequestType) + sizeof(unsigned);
			
			unsigned migrationSent = 0;

			for (unsigned i = 0; i < _objectMigrationOut.size(); ++i)
			{
				if (message.Size() + objectStride >= Message::MAX_BUFFER_SIZE)
				{
					if (!socket.Send(message))
					{
						_objectMigrationOut.erase(_objectMigrationOut.begin(), _objectMigrationOut.begin() + migrationSent);
						return;
					}

					message.Clear();
					message.Append(OBJECT_MIGRATION);
					migrationSent = i;
				}

				message.Append(_objectMigrationOut[i].type);
				message.Append(_objectMigrationOut[i].objectId);
			}

			if (migrationSent != _objectMigrationOut.size())
			{
				if (socket.Send(message))
				{
					_objectMigrationOut.clear();
				}
			}
		}
	}
}

void ObjectExchange::ExchangeUpdatesWithWorld()
{
	Threading::ScopedLock lock (_exchangeMutex);

	ProcessOwnershipRequests();

	ProcessOwnershipConfirmations();

	StoreNewPositionUpdates();

	ProcessReceivedPositionUpdates();
}

void ObjectExchange::GatherInitialisationData()
{
	unsigned numObjects = _worldThread._world->GetNumObjects();
	_initialisationDataOut._messagesSent = 0;
	const int objectStride = sizeof(unsigned) * 2 + sizeof(double) * 5;

	Message message;
	message.Append(numObjects);

	for (unsigned i = 0; i < numObjects; ++i)
	{
		// If another object can't fit into the message, add it onto the list
		// and start building another message
		if (message.Size() + objectStride > Message::MAX_BUFFER_SIZE)
		{
			_initialisationDataOut._messages.push_back(message);
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
		_initialisationDataOut._messages.push_back(message);
	}

	_sendData._messagesSent = 0;
}

void ObjectExchange::SendInitialisationData(TcpSocket& socket)
{
	while (_initialisationDataOut._messagesSent < _initialisationDataOut._messages.size())
	{
		// If sending would block, give up for now
		if (!socket.Send(_initialisationDataOut._messages[_initialisationDataOut._messagesSent]))
		{
			return;
		}
		
		 _initialisationDataOut._messagesSent++;
	}
}

bool ObjectExchange::InitialisationSent()
{
	return _initialisationDataOut._messagesSent == _initialisationDataOut._messages.size();
}

void ObjectExchange::ReceiveInitialisationData(TcpSocket& socket)
{
	Message message;
	while (socket.Receive(message) && socket.IsOpen())
	{
		if (_initialisationDataIn._objects.size() == 0)
		{
			int objectsToRead;
			if (!message.Read(objectsToRead))
			{
				socket.Close();
				return;
			}
			std::cout << "Recieving " << objectsToRead << " objects" << std::endl;
			_initialisationDataIn._objectsRead = 0;
			_initialisationDataIn._objects.resize(objectsToRead);
		}

		while(_initialisationDataIn._objectsRead < _initialisationDataIn._objects.size() 
					&& socket.IsOpen())
		{						
			ObjectInitialisation& objectInit = _initialisationDataIn._objects[_initialisationDataIn._objectsRead];
			objectInit.objectType = 0;

			// If reading an object failed exit and wait for next message
			if (!message.Read(objectInit.objectType))
			{
				return;
			}
			// Reading object type was successul so continue to read remainder of object
			else
			{
				assert(objectInit.objectType != 0);
				bool valid = true;

				valid &= message.Read(objectInit.x);
				valid &= message.Read(objectInit.y);
				valid &= message.Read(objectInit.vx);
				valid &= message.Read(objectInit.vy);
				valid &= message.Read(objectInit.color);
				valid &= message.Read(objectInit.mass);

				if (!valid)
				{
					socket.Close();
					return;
				}

				_initialisationDataIn._objectsRead++;
			}
		}

		if (_initialisationDataIn._objects.size() == _initialisationDataIn._objectsRead)
		{
			Threading::ScopedLock lock (_exchangeMutex);
			_initialisationDataIn._initialisationReceived = true;
			return;
		}
	}
}

bool ObjectExchange::InitialisationReceived()
{
	return _initialisationDataIn._initialisationReceived;
}

void ObjectExchange::ExchangeInitialisation()
{
	Threading::ScopedLock lock (_exchangeMutex);

	std::cout << "received " << _initialisationDataIn._objects.size() << "objects " << std::endl;
	_world.ClearObjects();

	for (unsigned i = 0; i < _initialisationDataIn._objects.size(); ++i)
	{
		Physics::PhysicsObject* object = NULL;
		const ObjectInitialisation& objectInit = _initialisationDataIn._objects[i];

		switch (objectInit.objectType)
		{
		case Physics::OBJECT_BOX:
			object = _world.AddBox();
			break;

		case Physics::OBJECT_TRIANGLE:
			object = _world.AddTriangle();
			break;

		case Physics::OBJECT_BLOBBY:
			object = _world.AddBlobbyObject();

			// When we find a blobby object it should be preceded by a sequence of blobby parts
			// which are created with the blobby object and so need to be updated
			for (int j = 0; j < 16; j++)
			{
				const ObjectInitialisation& blobbyPartInit = _initialisationDataIn._objects[i-Physics::BlobbyObject::NUM_PARTS+j];
				assert(blobbyPartInit.objectType == Physics::OBJECT_BLOBBY_PART);
				
				Physics::PhysicsObject* blobbyPart = _world.GetObject(_world.GetNumObjects() - Physics::BlobbyObject::NUM_PARTS + j - 1);
				assert(blobbyPart->GetSerializationType() == Physics::OBJECT_BLOBBY_PART);

				blobbyPart->SetPosition(Vector2d(blobbyPartInit.x, blobbyPartInit.y));
				blobbyPart->SetVelocity(Vector2d(blobbyPartInit.vx, blobbyPartInit.vy));
				blobbyPart->SetMass(blobbyPartInit.mass);
				blobbyPart->SetColor(Color(objectInit.color));
			}
			break;
		}

		if (object != NULL)
		{
			object->SetPosition(Vector2d(objectInit.x, objectInit.y));
			object->SetVelocity(Vector2d(objectInit.vx, objectInit.vy));
			object->SetMass(objectInit.mass);
			object->SetColor(Color(objectInit.color));
			object->UpdateShape(*_worldThread._world);
		}
	}

	_initialisationDataIn._objects.clear();
	_updateData._objectsRead = 0;
}

void ObjectExchange::StoreNewPositionUpdates()
{
	_sendData._newState.clear();

	int numObjects = _worldThread._world->GetNumObjects();

	// TODO: cache this somewhere
	int numObjectsOwned = 0;
	for (int i = 0; i < numObjects; ++i)
	{
		if (_peerId == _world.GetObject(i)->GetOwnerId())
		{
			numObjectsOwned++;
		}
	}

	const int objectStride = sizeof(unsigned) * 1 + sizeof(double) * 4;

	Message message;
	message.Append(OBJECT_UPDATES);
	message.Append(numObjectsOwned);

	int sent = 0;
	int appended = 0;
	for (int i = 0; i < numObjects; ++i)
	{
		// If another object can't fit into the message, add it onto the list
		// and start building another message
		if (message.Size() + objectStride > Message::MAX_BUFFER_SIZE)
		{
			_sendData._newState.push_back(message);
			assert(message.Size() != 6);
			sent = appended;
			message.Clear();
			message.Append(OBJECT_UPDATES);
		}

		Physics::PhysicsObject* object = _world.GetObject(i);

		if (object->GetOwnerId() == _peerId)
		{
			message.Append(i);
			message.Append(object->GetPosition().x());
			message.Append(object->GetPosition().y());
			message.Append(object->GetVelocity().x());
			message.Append(object->GetVelocity().y());
			appended++;
		}
	}

	// If there are some objects left over that don't make a complete message, send it
	if (sent != numObjectsOwned)
	{
		_sendData._newState.push_back(message);
		assert(message.Size() != 6);
	}
}

void ObjectExchange::ProcessReceivedPositionUpdates()
{
	// Process position updates
	for (unsigned i = 0; i < _updateData._objectsUpdate.size(); ++i)
	{
		// TODO: check object is not owned by us first?
		const ObjectState& objectState = _updateData._objectsUpdate[i];
		Physics::PhysicsObject* object = _world.GetObject(objectState.id);

		object->SetPosition(Vector2d(objectState.x, objectState.y));
		object->SetVelocity(Vector2d(objectState.vx, objectState.vy));
	}

	// Clear these updates to be sure we don't apply them again
	_updateData._objectsUpdate.clear();
}

void ObjectExchange::ProcessOwnershipConfirmations()
{
	// Take ownership of objects we received confirmation for
	for (unsigned i = 0; i < _objectMigrationIn.size(); ++i)
	{
		Physics::PhysicsObject* object = _world.GetObject(_objectMigrationIn[i].objectId);

		if (_objectMigrationIn[i].type == OBJECT_REQUEST)
		{
			unsigned otherId = 0;
			if (_peerId == 0) otherId = 1;

			// Acknowledge migration request of object can migrate or the object
			// does not have the spring attached
			if (object->CanMigrate() && object != _world.GetSelectedObject())
			{
				object->SetOwnerId(otherId);
			
				_objectMigrationIn[i].type = OBJECT_REQUEST_ACK;
				_objectMigrationOut.push_back(_objectMigrationIn[i]);
			}
			else
			{
				_objectMigrationIn[i].type = OBJECT_REQUEST_DENY;
				_objectMigrationOut.push_back(_objectMigrationIn[i]);
			}

		}
		else if (_objectMigrationIn[i].type == OBJECT_REQUEST_ACK)
		{
			object->SetOwnerId(_peerId);
		}
	}

	_objectMigrationIn.clear();
}

void ObjectExchange::ProcessOwnershipRequests()
{
	ObjectMigration migration;
	migration.type = OBJECT_REQUEST;

	// Make requests for any object in our side of the world
	for (int i = 0; i < _world.GetNumObjects(); ++i)
	{
		Physics::PhysicsObject* object = _world.GetObject(i);
		if (object->GetOwnerId() != _peerId)
		{
			if ((_peerId == 1 && object->GetPosition().x() > 0) || _peerId == 0 && object->GetPosition().x() < 0)
			{
				// Don't queue more than 500 requests
				if (_objectMigrationOut.size() > 500)
					return;

				migration.objectId = i;
				_objectMigrationOut.push_back(migration);
			}
		}
	}

	Physics::PhysicsObject* object = _world.GetSelectedObject();

	// If an object that doesn't belong to is is picked up, request it
	if (object != NULL)
	{
		if (object->GetOwnerId() != _peerId)
		{
			migration.objectId = object->GetId();
			_objectMigrationOut.push_back(migration);
		}
	}

	// TODO: request object with spring attached
}

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
	_state(LISTENING_CLIENT),
	_objectExchange(worldThread, 0),
	_hadPeerConnected(false)
{
	_broadcastListenSocket.Bind(BROADCAST_PORT);
	_broadcastListenSocket.SetBlocking(false);

	_tcpListenSocket.SetBlocking(false);

	_broadcastReplyMessage.Append(BROADCAST_REPLY_STRING);
	_broadcastReplyMessage.Append(TCPLISTEN_PORT);
}

void SessionMasterController::ExchangeState()
{
	Threading::ScopedLock lock(_stateChangeMutex);

	// Make a copy of the world state to send to the client
	if (_state == WAIT_ON_INITIALISATION_GATHER)
	{
		_objectExchange.GatherInitialisationData();

		_state = ACCEPTING_CLIENT;
	}
	// For now synchronise all objects
	else if (_state == SYNCHRONISE_CLIENT)
	{
		_objectExchange.ExchangeUpdatesWithWorld();
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

	// If we lost a client
	if (_hadPeerConnected && !_clientSocket.IsOpen())
	{
		_hadPeerConnected = false;
		UpdatePeerId();
	}
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
		Threading::ScopedLock lock(_stateChangeMutex);
		std::cout << "Client Connected " << address << std::endl;
		_clientSocket.SetBlocking(false);
		_hadPeerConnected = true;
		_objectExchange.Reset();
		UpdatePeerId();
		_state = WAIT_ON_INITIALISATION_GATHER;
	}

}

void SessionMasterController::SendSessionInitialization()
{
	_objectExchange.SendInitialisationData(_clientSocket);

	if (_objectExchange.InitialisationSent())
	{
		_state = SYNCHRONISE_CLIENT;
	}
}


void SessionMasterController::DoPeerConnectedTick()
{
	_objectExchange.SendState(_clientSocket);
	_objectExchange.ReceiveState(_clientSocket);
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
	_state(FINDING_HOST),
	_objectExchange(worldThread, 1),
	_hadPeerConnected(false)
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

	// If we lost a peer
	if (_hadPeerConnected && !_serverSocket.IsOpen())
	{
		_hadPeerConnected = false;
		_state = FINDING_HOST;
		UpdatePeerId();
	}
}

void WorkerController::UpdatePeerId()
{
	if (_state != FINDING_HOST)
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
	_objectExchange.SendState(_serverSocket);
	_objectExchange.ReceiveState(_serverSocket);
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
			Threading::ScopedLock lock(_stateChangeMutex);
			std::cout << "Connected, awaiting initialization data" << std::endl;
			_serverSocket.SetBlocking(false);
			_hadPeerConnected = true;
			_objectExchange.Reset();
			_state = RECEIVING_INITIALISATION;
			UpdatePeerId();
		}
		else
		{
			std::cout << "Connection failed" << std::endl;
		}
	}
}

void WorkerController::ReceiveInitialisationData()
{
	_objectExchange.ReceiveInitialisationData(_serverSocket);

	if (_objectExchange.InitialisationReceived())
	{
		Threading::ScopedLock lock(_stateChangeMutex);
		_state = RECEIVED_INITIALISATION;
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
	Threading::ScopedLock lock(_stateChangeMutex);

	if (_state == RECEIVED_INITIALISATION)
	{
		_objectExchange.ExchangeInitialisation();
		_state = SYNCHRONISE_CLIENT;
	}
	else if (_state == SYNCHRONISE_CLIENT)
	{
		_objectExchange.ExchangeUpdatesWithWorld();
	}
}
