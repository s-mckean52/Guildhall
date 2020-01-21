#pragma once
#include "Game/Entity.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Game/Game.hpp"

class Actor : public Entity
{
public:
	Actor( Game* theGame, Map* theMap, const Vec2& position, std::string actorTypeName );
	~Actor() {};

	virtual void	Update( float deltaSeconds ) override;

	virtual void	Render() const override;
	void			RenderBase() const;

	virtual void	TakeDamage( int damageDealt ) override;
	virtual void	Die() override;

private:
	void			UpdateFromKeyboard( float deltaSeconds );
	void			UpdateFromJoystick( float deltaSeconds );
	void			Respawn();
	
	EntityFaction	SetFactionBasedOnXml( std::string param1 );

private:
	Game*	m_theGame = nullptr;
	Vec2	m_startPosition;
	int		m_defaultSpriteIndex = 0;
	int		m_controllerID = -1;
	float	m_thrustFraction = 0.f;
};