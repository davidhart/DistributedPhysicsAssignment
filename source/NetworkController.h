#pragma once

#include "Networking.h"
#include "Uncopyable.h"
#include "Threading.h"
#include "AABB.h"
#include <vector>
#include <queue>

class GameWorldThread;
class World;


enum eMessageType
{
	OBJECT_UPDATES = 0,
	OBJECT_MIGRATION = 1,
};

enum eRequestType
{
	OBJECT_REQUEST = 0,
	OBJECT_REQUEST_ACK = 1,
	OBJECT_REQUEST_DENY = 2,
};

struct ObjectMigration
{
	eRequestType type;
	unsigned objectId;
};

struct ObjectInitialisation
{
	unsigned objectType;
	double x;
	double y;
	double vx;
	double vy;
	int color;
	double mass;
};

struct ObjectState
{
	unsigned id;
	double x;
	double y;
	double vx;
	double vy;
};

class ObjectExchange
{

public:

	ObjectExchange(GameWorldThread& worldThread, unsigned peerId);

	void SendState(Networking::TcpSocket& socket);
	void ReceiveState(Networking::TcpSocket& socket);

	void ExchangeUpdatesWithWorld();

	void GatherInitialisationData();
	void SendInitialisationData(Networking::TcpSocket& socket);
	bool InitialisationSent();

	void ReceiveInitialisationData(Networking::TcpSocket& socket);
	bool InitialisationReceived();

	void ExchangeInitialisation();

	void Reset();

private:

	void HandleUpdateMessage(Networking::TcpSocket& socket, Networking::Message& message);
	void HandleMigrationMessage(Networking::TcpSocket& socket, Networking::Message& message);

	void StoreNewPositionUpdates();
	void ProcessReceivedPositionUpdates();
	void ProcessOwnershipConfirmations();
	void ProcessOwnershipRequests();

	Threading::Mutex _exchangeMutex;

	std::vector<ObjectMigration> _objectMigrationOut;
	std::vector<ObjectMigration> _objectMigrationIn;

	struct
	{
		std::vector<Networking::Message> _newState;
		std::vector<Networking::Message> _sendingState;
		unsigned _messagesSent;

	} _sendData;

	struct
	{
		unsigned _objectsRead;
		std::vector<ObjectState> _objectsReceived;
		std::vector<ObjectState> _objectsUpdate;

		AABB _peerBoundsReceived;
		AABB _peerBoundsUpdate;

	} _updateData;
	
	// Variables capturing initialisation data to send to peer
	struct 
	{
		std::vector<Networking::Message> _messages;
		unsigned _messagesSent;

	} _initialisationDataOut;

	// Variables capturing initialisation data as it arrives
	struct
	{
		unsigned _objectsRead;
		std::vector<ObjectInitialisation> _objects;
		bool _initialisationReceived;

	} _initialisationDataIn;

	World& _world;
	GameWorldThread& _worldThread;

	unsigned _peerId;
};

class NetworkController : public Threading::Thread
{
	
public:

	NetworkController();
	virtual ~NetworkController();
	void Shutdown();
	virtual void ExchangeState() = 0;

protected:

	static const unsigned short BROADCAST_PORT = 7777;
	static const unsigned short TCPLISTEN_PORT = 2869;

	static const std::string BROADCAST_STRING;
	static const std::string BROADCAST_REPLY_STRING;

	virtual void DoTick() = 0;

private:

	unsigned ThreadMain();
	
	bool _shutDown;

};

class SessionMasterController : public NetworkController
{

public:

	SessionMasterController(GameWorldThread& worldThread);
	void ExchangeState();

protected:

	void DoTick();

private:

	void DoAcceptHostTick();
	void DoPeerConnectedTick();
	void UpdatePeerId();
	void SendSessionInitialization();

	Networking::UdpSocket _broadcastListenSocket;
	Networking::TcpListener _tcpListenSocket;

	Networking::TcpSocket _clientSocket;
	Networking::Message _broadcastReplyMessage;

	GameWorldThread& _worldThread;
	Threading::Mutex _stateChangeMutex;

	enum eState
	{
		LISTENING_CLIENT,
		WAIT_ON_INITIALISATION_GATHER,
		ACCEPTING_CLIENT,
		SYNCHRONISE_CLIENT,
		DROPPING_CLIENT,
	};

	volatile eState _state;

	ObjectExchange _objectExchange;

	bool _hadPeerConnected;
};

class WorkerController : public NetworkController
{

public:

	WorkerController(GameWorldThread& worldThread);
	void ExchangeState();

protected:

	void DoTick();

private:

	void UpdatePeerId();
	void DoConnectedTick();
	void DoFindHostTick();
	void ReceiveInitialisationData();

	bool ValidateBroadcastResponseAndGetPort(Networking::Message& message, unsigned short& port);

	void CloseConnection();

	Networking::UdpSocket _broadcastSocket;
	Networking::Message _broadcastMessage;
	Networking::TcpSocket _serverSocket;

	GameWorldThread& _worldThread;

	enum eState
	{
		FINDING_HOST,
		RECEIVING_INITIALISATION,
		RECEIVED_INITIALISATION,
		SYNCHRONISE_CLIENT,
		DROPPING_CLIENT,
	};

	volatile eState _state;

	ObjectExchange _objectExchange;

	Threading::Mutex _stateChangeMutex;

	bool _hadPeerConnected;
};