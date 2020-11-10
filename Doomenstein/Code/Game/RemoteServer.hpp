#pragma once
#include "Game/Server.hpp"


class RemoteServer : public Server
{
public:
	RemoteServer( std::string const& ipAddress, std::string const& port );
	~RemoteServer() {};

	virtual void StartUp( GameType gameType )	override;
	virtual void ShutDown()						override;
	virtual void BeginFrame()					override;
	virtual void EndFrame()						override;
	virtual void Update()						override;

	void ProcessTCPMessage( TCPMessage* messageToProcess );
	void OpenUDPSocket( std::string const& udpSocketData );

private:
	uint16_t	m_key			= 0;
	uint16_t	m_udpListenPort = 48000;
	uint16_t	m_udpSendPort	= 480001;
	std::string m_connectionIP	= "127.0.0.1";
};