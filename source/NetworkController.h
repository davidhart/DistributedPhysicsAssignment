#pragma once

#include "Networking.h"
#include "Uncopyable.h"
#include "Threading.h"
#include <vector>

class GameWorldThread;

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

class NetworkController : public Threading::Thread
{
	
public:

	NetworkController();
	virtual ~NetworkController();
	void Shutdown();
	virtual void ExchangeState() = 0;

protected:

	static const unsigned short BROADCAST_PORT = 7777;
	static const unsigned short TCPLISTEN_PORT = 1234;

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
	Threading::Mutex _exchangeMutex;

	enum eState
	{
		LISTENING_CLIENT,
		WAIT_ON_INITIALISATION_GATHER,
		ACCEPTING_CLIENT,
		SYNCHRONISE_CLIENT,
	};

	volatile eState _state;

	struct 
	{
		std::vector<Networking::Message> _messages;
		unsigned _messagesSent;

	} _initialisationData;

	struct
	{
		std::vector<Networking::Message> _newState;
		std::vector<Networking::Message> _sendingState;
		unsigned _messagesSent;
	} _sendData;
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

	Threading::Mutex _exchangeMutex;

	enum eState
	{
		FINDING_HOST,
		RECEIVING_INITIALISATION,
		RECEIVED_INITIALISATION,
		SYNCHRONISE_CLIENT,
	};

	volatile eState _state;

	// Variables capturing initialisation data as it arrives
	struct
	{
		unsigned _objectsRead;

		std::vector<ObjectInitialisation> _objects;

	} _initialisationData;

	struct
	{
		unsigned _objectsRead;

		std::vector<ObjectState> _objectsReceived;

		std::vector<ObjectState> _objectsUpdate;

	} _updateData;
};