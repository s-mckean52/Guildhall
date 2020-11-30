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

	void SendInputData();
	void SendDisconnectMessage();
	void RequestConnectionData();

	void ProcessUDPMessages();
	void ProcessUDPMessage( UDPMessage const& message );
	
	void UnpackUDPMessage( UDPMessage const& message );

	void OpenUDPSocket(TCPMessage const& messageToProcess)	override;
	void ProcessDisconnect();
	void ProcessInputData( UDPMessage const& message );
	void ProcessEntityData( UDPMessage const& message );
	void ProcessConnectionData( UDPMessage const& message );
	void ProcessCameraData( UDPMessage const& message );

private:
	uint16_t	m_udpListenPort = 48000;
	uint16_t	m_udpSendPort = 48001;
	std::string m_connectionIP = "127.0.0.1";

	uint16_t m_key = 0;
	UDPSocket* m_socket = nullptr;
	UDPPacket m_packets[NUM_MESSAGE_ID] = {};
};