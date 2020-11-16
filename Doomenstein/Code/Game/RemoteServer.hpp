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
	void RequestConnectionData();

	void OpenUDPSocket( TCPMessage const& messageToProcess )	override;
	void ProcessInputData( UDPMessage const& message )			override;
	void ProcessEntityData( UDPMessage const& message )			override;
	void ProcessConnectionData( UDPMessage const& message )		override;
	void ProcessCameraData( UDPMessage const& message )			override;

private:
	uint16_t	m_key			= 0;
};