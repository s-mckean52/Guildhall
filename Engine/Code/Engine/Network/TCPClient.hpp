#pragma once

#include "Engine/Network/TCPSocket.hpp"
#include "Engine/Core/EventSystem.hpp"

class TCPClient
{
	friend class NetworkSystem;

protected:
	TCPClient();

public:
	~TCPClient();
	
	TCPSocket Connect( std::string const& host, uint16_t port, SocketMode mode = SocketMode::BLOCKING );

private:
	SocketMode m_mode = SocketMode::INVALID;
};