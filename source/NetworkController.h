#pragma once

#include "Networking.h"

class GameWorldThread;

class NetworkController
{
	
public:

	virtual ~NetworkController();
	virtual void DoTick() = 0;


	static const unsigned short BROADCAST_PORT = 7777;
	static const unsigned short TCPLISTEN_PORT = 1234;

	static const std::string BROADCAST_STRING;
	static const std::string BROADCAST_REPLY_STRING;

};

class SessionMasterController : public NetworkController
{

public:

	SessionMasterController(GameWorldThread& worldThread);
	~SessionMasterController();

	void DoTick();

private:

	void SendSessionInitialization();

	Networking::UdpSocket _broadcastListenSocket;
	Networking::TcpListener _tcpListenSocket;

	Networking::TcpSocket _clientSocket;
	Networking::Message _broadcastReplyMessage;

	GameWorldThread& _worldThread;
};

class WorkerController : public NetworkController
{

public:

	WorkerController(GameWorldThread& worldThread);
	~WorkerController();
	void DoTick();

private:

	void DoConnectedTick();
	void DoFindHostTick();
	void ReceiveInitialisationData();

	bool ValidateBroadcastResponseAndGetPort(Networking::Message& message, unsigned short& port);

	Networking::UdpSocket _broadcastSocket;
	Networking::Message _broadcastMessage;
	Networking::TcpSocket _serverSocket;

	GameWorldThread& _worldThread;
};