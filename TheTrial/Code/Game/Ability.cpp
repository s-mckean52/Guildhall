#include "Game/Ability.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include <vector>

//---------------------------------------------------------------------------------------------------------
Ability::Ability( Game* theGame, Entity* owner, double cooldownSeconds, std::string const& name )
{
	m_theGame = theGame;
	m_owner = owner;
	m_name = name;
	m_baseCooldownSeconds = cooldownSeconds;

	m_cooldownTimer = new Timer();
	m_cooldownTimer->SetSeconds( theGame->GetGameClock(), 0.0 );
}


//---------------------------------------------------------------------------------------------------------
Ability::~Ability()
{

}


//---------------------------------------------------------------------------------------------------------
void Ability::Update()
{
}


//---------------------------------------------------------------------------------------------------------
void Ability::Render() const
{
	AABB2 abilityBounds( 5.f, 0.1f, 6.f, 1.1f );

	std::vector<Vertex_PCU> abilityVerts;
	g_theRenderer->BindMaterial( nullptr );
	AppendVertsForAABB2D( abilityVerts, abilityBounds, Rgba8::WHITE );
	g_theRenderer->DrawVertexArray( abilityVerts );

	std::string abilityStatusString = "Ready";
	if( IsOnCooldown() )
	{
		double elapsedTime = m_cooldownTimer->GetSecondsRemaining();
		abilityStatusString = Stringf( "%.1f", elapsedTime );
	}
	
	std::vector<Vertex_PCU> abilityTextVerts;
	g_devConsoleFont->AddVertsForTextInBox2D( abilityTextVerts, abilityBounds, 0.1f, abilityStatusString, Rgba8::BLUE );
	g_devConsoleFont->AddVertsForTextInBox2D( abilityTextVerts, abilityBounds, 0.1f, m_name, Rgba8::BLUE, 1.f, ALIGN_TOP_CENTERED );
	g_theRenderer->BindTexture( g_devConsoleFont->GetTexture() );
	g_theRenderer->DrawVertexArray( abilityTextVerts );
}


//---------------------------------------------------------------------------------------------------------
void Ability::Use()
{
	double cooldownSeconds = m_baseCooldownSeconds;
	m_cooldownTimer->SetSeconds( m_theGame->GetGameClock(), cooldownSeconds );
}


//---------------------------------------------------------------------------------------------------------
bool Ability::IsOnCooldown() const
{
	return !m_cooldownTimer->HasElapsed();
}


