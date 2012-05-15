// David Hart - 2012

#include "Networking.h"

#include <iostream>
#include <cassert>

using namespace Networking;

int System::INSTANCE_COUNT = 0;
WSADATA WSA_DATA;

System::System()
{
	if (INSTANCE_COUNT == 0)
	{
		int r = WSAStartup(MAKEWORD(2, 2), &WSA_DATA);

		assert(r == 0);

		INSTANCE_COUNT++;
	}
}

System::~System()
{
	INSTANCE_COUNT--;

	if (INSTANCE_COUNT == 0)
		WSACleanup();
}

void System::PrintLastError()
{
	std::cout << WSAGetLastError() << std::endl;
}

Message::Message() :
	_size(0),
	_readLocation(2)
{
	AllocateSizePrefix();
}

void Message::Append(const void* buffer, unsigned size)
{
	assert(_size + size <= MAX_BUFFER_SIZE);
	memcpy_s(_size + _buffer, size, buffer, size);
	_size += (unsigned short)size;
}

bool Message::Read(void* data, unsigned size)
{
	if (_readLocation + size > _size)
	{
		return false;
	}

	memcpy_s(data, size, _buffer+_readLocation, size);
	_readLocation += (unsigned short)size;

	return true;
}


void Message::Clear()
{
	_readLocation = sizeof(unsigned short);
	_size = sizeof(unsigned short);
}

bool Message::ReadString(std::string& string, unsigned short length)
{
	int end = _readLocation + length;

	if (end > _size)
	{
		return false;
	}

	string.clear();
	string.reserve(length);

	for (; _readLocation < end; _readLocation++)
	{
		string.push_back(_buffer[_readLocation]);
	}
	return true;
}

void Message::AllocateSizePrefix()
{
	unsigned short size = 0;
	Append(size);
}

char* Message::Data() const
{
	*(unsigned short*)_buffer = _size;

	return _buffer;
}

short Message::Size() const
{
	return _size;
}

Address::Address()
{
}

Address::Address(const sockaddr_in& addr) : 
	_addr(addr)
{
}

void Address::SetPort(unsigned short port)
{
	_addr.sin_port = htons(port);
}

std::ostream& Address::Write(std::ostream& out) const
{
	out << inet_ntoa(_addr.sin_addr) << ":" << ntohs(_addr.sin_port);

	return out;
}

std::ostream& operator<<(std::ostream& out, const Address& address)
{
	return address.Write(out);
}

UdpSocket::UdpSocket() :
	_bound(false)
{
	_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

UdpSocket::~UdpSocket()
{
	if (_socket != INVALID_SOCKET)
	{
		closesocket(_socket);
	}
}

void UdpSocket::Bind(unsigned short port)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = INADDR_ANY;
	addr.sin_port = htons(port);

	int r = bind(_socket, (sockaddr*)&addr, sizeof(addr));

	_bound = r != SOCKET_ERROR;
}

void UdpSocket::Broadcast(Message& message, unsigned short port)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.S_un.S_addr = htonl(INADDR_BROADCAST);

	BOOL opt = TRUE;
	setsockopt(_socket, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(opt));

	int r = sendto(_socket, message.Data(), message.Size(), 0, (sockaddr*)&addr, sizeof(addr));

	if (r == SOCKET_ERROR)
	{
		System::PrintLastError();
	}
}

bool UdpSocket::RecieveFrom(Address& address, Message& message)
{
	sockaddr_in addr;
	int s = sizeof(addr);
	char buffer[Message::MAX_BUFFER_SIZE];

	int r = recvfrom(_socket, buffer, Message::MAX_BUFFER_SIZE, 0, (sockaddr*)&addr, &s);

	if (r != SOCKET_ERROR)
	{
		message.Clear();

		unsigned short messageSize = *(unsigned short*)buffer;

		if (messageSize <= Message::MAX_BUFFER_SIZE)
		{
			address = Address(addr);
			message.Append(buffer + sizeof(unsigned short), messageSize - sizeof(unsigned short));
			return true;
		}
	}
	else if (WSAGetLastError() != WSAEWOULDBLOCK)
	{
		System::PrintLastError();
	}

	return false;
}

void UdpSocket::SetBlocking(bool blocking)
{
	u_long arg = !blocking;
	ioctlsocket(_socket, FIONBIO, &arg);
}

void UdpSocket::SendTo(Address& address, Message& message)
{
	int r = sendto(_socket, message.Data(), message.Size(), 0, (sockaddr*)&(address._addr), sizeof(address._addr));

	if (r == SOCKET_ERROR)
	{
		System::PrintLastError();
	}
}

TcpSocket::TcpSocket() :
	_socket(INVALID_SOCKET),
	_bytesRead(0),
	_messageStart(0)
{
}

TcpSocket::TcpSocket(Address& address) :
	_bytesRead(0),
	_messageStart(0)
{
	_socket = socket(AF_INET, INADDR_ANY, IPPROTO_TCP);

	if (_socket != INVALID_SOCKET)
	{
		int r = connect(_socket, (sockaddr*)&(address._addr), sizeof(address._addr));

		if (r == SOCKET_ERROR)
		{
			System::PrintLastError();
			Close();
		}
	}
}

bool TcpSocket::Send(Message& message)
{
	int r = send(_socket, message.Data(), message.Size(), 0);

	if (r == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
		{
			System::PrintLastError();
			Close();
		}
		return false;
	}

	return true;
}

bool TcpSocket::Receive(Message& message)
{
	// Edge case, we have 1 byte of a message in the middle of the buffer
	if (_bytesRead == 1)
	{
		_buffer[0] = _buffer[_messageStart];
		_messageStart = 0;
	}

	// Read at least one whole message
	while(IsOpen())
	{
		unsigned expectedSize = Message::MAX_BUFFER_SIZE;

		// If we already have a whole message, return it
		if (_bytesRead >= 2)
		{
			expectedSize = *(unsigned short*)(_buffer + _messageStart);

			if (_bytesRead >= expectedSize)
			{
				message.Clear();
				message.Append(_buffer + _messageStart + sizeof(unsigned short), 
					expectedSize - sizeof(unsigned short));

				_bytesRead -= expectedSize;

				if (_bytesRead == 0)
				{
					_messageStart = 0;
				}
				else
				{
					_messageStart += expectedSize;
				}

				return true;
			}
		}

		int r = recv(_socket, _buffer + _messageStart + _bytesRead, expectedSize - _bytesRead, 0);

		if (r == SOCKET_ERROR)
		{
			if (WSAGetLastError() !=  WSAEWOULDBLOCK)
			{
				System::PrintLastError();
				Close();
			}
			return false;
		}

		_bytesRead += r;
	}

	return false;
}

void TcpSocket::SetTimeout(unsigned int milliseconds)
{
	if (_socket == INVALID_SOCKET)
		return;

	DWORD ms = milliseconds;
	int r = setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&ms, sizeof(ms));

	if (r == SOCKET_ERROR)
	{
		System::PrintLastError();
		Close();
	}

	r = setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&ms, sizeof(ms));

	if (r == SOCKET_ERROR)
	{
		System::PrintLastError();
		Close();
	}
}

void TcpSocket::SetBlocking(bool blocking)
{
	u_long arg = !blocking;
	ioctlsocket(_socket, FIONBIO, &arg);
}

void TcpSocket::Close()
{
	if (_socket != INVALID_SOCKET)
	{
		closesocket(_socket);
		_socket = INVALID_SOCKET;
	}
}

bool TcpSocket::IsOpen()
{
	return _socket != INVALID_SOCKET;
}

TcpSocket::TcpSocket(SOCKET socket) :
	_socket(socket),
	_messageStart(0),
	_bytesRead(0)
{
}

TcpListener::TcpListener(unsigned short port)
{
	_socket = socket(AF_INET, INADDR_ANY, IPPROTO_TCP);

	if (_socket != INVALID_SOCKET)
	{
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.S_un.S_addr = INADDR_ANY;

		int r = bind(_socket, (sockaddr*)&addr, sizeof(addr));

		if (r == SOCKET_ERROR)
		{
			System::PrintLastError();
		}

		r = listen(_socket, 10);

		if (r == SOCKET_ERROR)
		{
			System::PrintLastError();
		}
	}
}

TcpListener::~TcpListener()
{
	if (_socket != INVALID_SOCKET)
	{
		closesocket(_socket);
	}
}

bool TcpListener::Accept(TcpSocket& socket, Address& address)
{
	sockaddr_in addr;
	int size = sizeof(addr);
	SOCKET s = accept(_socket, (sockaddr*)&addr, &size);  

	socket = TcpSocket(s);

	if (s != INVALID_SOCKET)
	{
		address = Address(addr);
		return true;
	}
	else if (WSAGetLastError() != WSAEWOULDBLOCK)
	{
		System::PrintLastError();
	}

	return false;
}

void TcpListener::SetBlocking(bool blocking)
{
	u_long arg = !blocking;
	int r = ioctlsocket(_socket, FIONBIO, &arg);
	if (r == SOCKET_ERROR)
	{
		System::PrintLastError();
	}
}