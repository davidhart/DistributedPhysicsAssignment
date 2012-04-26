#pragma once

#include <WinSock2.h>
#include <string>
#include <iosfwd>

namespace Networking
{

	class System
	{

	public:

		System();
		~System();

		static void PrintLastError();

	private:

		static int INSTANCE_COUNT;

	};

	class Message
	{

	public:
		Message();

		char* Data() const;
		short Size() const;

		void Clear();
		void Append(const void* buffer, unsigned size);
		template <typename T> void Append(const T& value);
		template <typename T> bool Read(T& value);

		static const int MAX_BUFFER_SIZE = 512;

	private:

		bool Read(void* data, unsigned length);
		bool ReadString(std::string& string, unsigned short length);
		void AllocateSizePrefix();
		mutable char _buffer[MAX_BUFFER_SIZE];
		unsigned short _size;
		unsigned short _readLocation;
		
	};

#include "Message.inl"

	class Address
	{
		friend class UdpSocket;
		friend class TcpSocket;
		friend class TcpListener;

	public:

		Address();
		// Address(const std::string& ip, unsigned short port);

		void SetPort(unsigned short port);

		std::ostream& Write(std::ostream& out) const;

	private:

		Address(const sockaddr_in& addr);
		sockaddr_in _addr;

	};

	class UdpSocket
	{

	public:

		UdpSocket();
		~UdpSocket();

		void Bind(unsigned short port);
		void SendTo(Address& address, Message& message);
		bool RecieveFrom(Address& address, Message& message);
		void Broadcast(Message& message, unsigned short port);
		void SetBlocking(bool blocking);

	private:

		SOCKET _socket;
		bool _bound;

	};

	class TcpSocket
	{
		friend class TcpListener;

	public:

		TcpSocket();
		TcpSocket(Address& address);

		bool Send(Message& message);
		bool Receive(Message& message);

		void SetTimeout(unsigned int milliseconds);

		void SetBlocking(bool blocking);

		void Close();
		bool IsOpen();

	private:

		TcpSocket(SOCKET socket);

		SOCKET _socket;

		char _buffer[Message::MAX_BUFFER_SIZE*2];
		unsigned _bytesRead;
		unsigned _messageStart;

	};

	class TcpListener
	{

	public:

		TcpListener(unsigned short port);
		~TcpListener();

		bool Accept(TcpSocket& socket, Address& address);
		void SetBlocking(bool blocking);

	private:

		SOCKET _socket;

	};

};


std::ostream& operator<<(std::ostream& out, const Networking::Address& address);