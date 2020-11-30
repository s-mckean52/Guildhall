#pragma once

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#include "Engine/Core/SynchronizedNonBlockingQueue.hpp"
#include "Engine/Network/NetworkMessages.hpp"
#include <WinSock2.h>
#include <limits>
#include <string>
#include <array>
#include <thread>
#include <deque>
constexpr int BufferSize = 512;
typedef std::array<char, BufferSize> Buffer;


class NetworkSystem;

class UDPSocket
{
public:

public:
	UDPSocket( NetworkSystem* owner, std::string const& host, int receievePort, int sendToPort );
	~UDPSocket();

	void	SendMessage( UDPMessage const& message, bool isOldMessage = false );
	bool	ReadMessage( UDPMessage& message );
	void	GetMessages( std::deque<UDPMessage>& out_messages );

	void	SendReliableMessages();
	void	RemoveReliableMessage( UDPMessage const& message );

	bool	IsValid() const { return m_socket != INVALID_SOCKET; }

	void	Close();
	void	StopThreads();
	void	Bind( int port );
	int		Send( int length );
	int		Receive();

	std::string	GetHostData();
	uint16_t	GetSendToPort() { return m_sendToPort; }
	uint16_t	GetReceivePort() { return m_receivePort; }

	Buffer& SendBuffer()	{ return m_sendBuffer; }
	Buffer& ReceiveBuffer() { return m_recieveBuffer; }

private:
	void UDPReceiveMessagesJob();
	void UDPSendMessagesJob();

private:
	NetworkSystem* m_owner = nullptr;

	bool		m_isUDPSocketQuitting = false;
	std::thread m_readThread;
	std::thread m_sendThread;
	SynchronizedNonBlockingQueue<UDPMessage> m_UDPMessagesToReceive;
	SynchronizedNonBlockingQueue<UDPMessage> m_UDPMessagesToSend;

	std::vector<UDPMessage> m_reliableMessages;

	uint16_t m_sendToPort = 0;
	uint16_t m_receivePort = 0;

	Buffer		m_sendBuffer;
	Buffer		m_recieveBuffer;
	sockaddr_in m_toAddress;
	sockaddr_in m_bindAddress;
	SOCKET		m_socket		= INVALID_SOCKET;
};