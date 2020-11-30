#pragma once
#include "Game/Client.hpp"

class Server;

class PlayerClient : public Client
{
public:
	PlayerClient( Server* owner );
	~PlayerClient();

	void BeginFrame()	override;
	void EndFrame()		override;
	void Update()		override;
	void Render()		override;

	void SendReliableWorldData() override;
};