#pragma once
#include "Game/Client.hpp"

class Server;

class RemoteClient : public Client
{
public:
	RemoteClient( Server* owner );
	~RemoteClient();

	void BeginFrame()	override;
	void EndFrame()		override;
	void Update()		override;
	void Render()		override;
};