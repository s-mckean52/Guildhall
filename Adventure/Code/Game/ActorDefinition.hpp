#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include <map>
#include <string>

class SpriteAnimDefinition;
class SpriteSheet;

class ActorDefinition
{
public:
	int			GetDefaultSpriteIndex() const	{ return m_defaultSpriteIndex; }
	std::string	GetName() const					{ return m_name; }
	std::string	GetFaction() const				{ return m_faction; }
	float		GetSpeed() const				{ return m_speed; }
	float		GetPhysicsRadius() const		{ return m_physicsRadius; }
	int			GetStartHealth() const			{ return m_startHealth; }
	int			GetMaxHealth() const			{ return m_maxHealth; }
	AABB2		GetSpriteBounds() const			{ return m_spriteBounds; }


private:
	explicit ActorDefinition( const XmlElement& element );

public:
	static std::map< std::string, ActorDefinition* > s_actorDefinitions;
	static void InitializeActorDefinitions();

private:
	int				m_defaultSpriteIndex	= 0;
	std::string		m_name					= "";
	std::string		m_faction				= "";
	float			m_speed					= 1.f;
	float			m_physicsRadius			= 1.f;
	int				m_startHealth			= 1;
	int				m_maxHealth				= 1;
	AABB2			m_spriteBounds;

	IntVec2					m_spriteInSheet = IntVec2();
	SpriteSheet*			m_spriteSheet	= nullptr;
	SpriteAnimDefinition*	m_idleAnim		= nullptr;
	SpriteAnimDefinition*	m_moveNorthAnim	= nullptr;
	SpriteAnimDefinition*	m_moveSouthAnim	= nullptr;
	SpriteAnimDefinition*	m_moveEastAnim	= nullptr;
	SpriteAnimDefinition*	m_moveWestAnim	= nullptr;
};