#pragma once
#include "Game/Server.hpp"


class AuthoritativeServer : public Server
{
public:
	void StartUp( GameType gameType )	override;
	void ShutDown()						override;
	void BeginFrame()					override;
	void EndFrame()						override;
	void Update()						override;

	void OpenUDPSocket( TCPMessage const& message )			override;
	void ProcessInputData( UDPMessage const& message )		override;
	void ProcessEntityData( UDPMessage const& message )		override;
	void ProcessConnectionData( UDPMessage const& message )	override;
	void ProcessCameraData( UDPMessage const& message )		override;

	void SendWorldDataToClient( Client* client );
	void SendSetupMessage( Client* client );
	void SendCameraData( Client* client );
};