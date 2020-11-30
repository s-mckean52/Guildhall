#pragma once
#include "Engine/Network/NetworkSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Game/GameCommon.hpp"
#include <vector>

class Game;
class Client;

enum MessageID
{
	MESSAGE_ID_DISCONNECT = 3,
	MESSAGE_ID_UDP_SOCKET = 4,
	MESSAGE_ID_INPUT_DATA = 5,
	MESSAGE_ID_ENTITY_DATA = 6,
	MESSAGE_ID_CONNECTION_DATA = 7,
	MESSAGE_ID_CAMERA_DATA = 8,

	NUM_MESSAGE_ID
};


class Server
{
public:
	Server();
	virtual ~Server();

	Game* GetGame() const;

	void AddClient( Client* remoteClient );
	bool IsValidMessage( int identifierToCompare ) { return m_identifier == identifierToCompare; }
	void SendLargeUDPData( UDPSocket* socket, std::string const& ipAddress, int sendToPort, void const* data, uint dataSize, MessageID messageType, uint frameNum, bool isReliable = false );
	void ProcessTCPMessages();
	void ProcessTCPMessage( TCPMessage const& message );

	void SendReliableUpdates();

	virtual void StartUp( GameType gameType )	= 0;
	virtual void ShutDown()						= 0;
	virtual void BeginFrame()					= 0;
	virtual void EndFrame()						= 0;
	virtual void Update()						= 0;

	virtual void OpenUDPSocket( TCPMessage const& message )			= 0;

protected:
	int m_frameNum = 0;
	int m_identifier = 0;
	
	std::vector<Client*> m_clients;
};