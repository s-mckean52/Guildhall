#include "Game/PlayerClient.hpp"
#include "Game/Server.hpp"
#include "Game/Game.hpp"

//---------------------------------------------------------------------------------------------------------
PlayerClient::PlayerClient( Server* owner )
	: Client( owner )
{
	owner->AddClient( this );
}


//---------------------------------------------------------------------------------------------------------
PlayerClient::~PlayerClient()
{
}


//---------------------------------------------------------------------------------------------------------
void PlayerClient::BeginFrame()
{
}


//---------------------------------------------------------------------------------------------------------
void PlayerClient::EndFrame()
{
	Render();
}


//---------------------------------------------------------------------------------------------------------
void PlayerClient::Update()
{
// 	Game* theGame = m_owner->GetGame();
// 	if (theGame != nullptr)
// 	{
// 		theGame->Update();
// 	}
}


//---------------------------------------------------------------------------------------------------------
void PlayerClient::Render()
{
	Game* theGame = m_owner->GetGame();
	if( theGame != nullptr )
	{
		theGame->Render();
	}
}

//---------------------------------------------------------------------------------------------------------
void PlayerClient::SendReliableWorldData()
{
}

