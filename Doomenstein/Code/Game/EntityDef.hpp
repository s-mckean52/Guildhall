#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include <string>
#include <map>

class SpriteSheet;
class SpriteDefinition;
class SpriteAnimSet;
class SpriteAnimDefinition;
struct Texture;


enum EntityType
{
	ENTITY_TYPE_ENTITY,
	ENTITY_TYPE_ACTOR,
	ENTITY_TYPE_PROJECTILE,
	ENTITY_TYPE_PORTAL,

	NUM_ENTITY_DEF_TYPES,

	INVALID_ENTITY_TYPE,
};

typedef	std::map<std::string, SpriteAnimSet*> SpriteAnimMap;


//---------------------------------------------------------------------------------------------------------
class EntityDef
{
private:
	EntityDef( XmlElement const& xmlElement, EntityType entityType );

public:
	std::string		GetName() const				{ return m_name; }
	EntityType		GetEntityType() const		{ return m_entityType; }
	float			GetRadius() const			{ return m_physicsRadius; }
	float			GetHeight()	const			{ return m_height; }
	float			GetEyeHeight() const		{ return m_eyeHeight; }
	float			GetSpeed() const			{ return m_speed; }
	Vec2 const&		GetSize() const				{ return m_size; }
	SpriteSheet*	GetSpriteSheet() const		{ return m_spriteSheet; }
	BillboardType	GetBillBoardType() const	{ return m_billboardType; }

	bool			HasAnimationDirectionInSet( std::string const& animationSet, std::string const& directionName ) const;

	std::string				GetEntityTypeAsString() const;
	SpriteDefinition const*	GetSpriteDefinitionForAnimSetAtTime( std::string const& animSetName, std::string const& direction, float time ) const;

private:
	//XML
	bool					ParsePhysicsNode( XmlElement const& element );
	bool					ParseAppearanceNode( XmlElement const& element );
	bool					CreateAnimState( XmlElement const& element );
	SpriteAnimDefinition*	CreateAnimDefinition( XmlElement const& element, char const* attributeName );
	bool					IsSupportedAnimDefType( std::string const& animDefType );
	

public:
	static void			CreateEntityDefsFromXML( char const* filepath );
	static SpriteSheet*	GetOrCreateEntitySpriteSheet( char const* filepath, IntVec2 const& layout );
	static EntityDef*	GetEntityDefByName( std::string const& entityName );

	//---------------------------------------------------------------------------------------------------------
	static EntityType GetEntityTypeFromString( std::string entityTypeAsString );

public:
	static std::map<std::string, EntityDef*>	s_entityDefs;
	static std::map<std::string, SpriteSheet*>	s_spriteSheets;


private:
	std::string	m_name			= "";
	EntityType	m_entityType	= ENTITY_TYPE_ENTITY;	

	//Physics
	float m_physicsRadius	= 10.f;
	float m_height			= 20.f;
	float m_eyeHeight		= 15.f;
	float m_speed			= 1.f;

	//Appearance
	Vec2			m_size			= Vec2( 20.f, 20.f );
	BillboardType	m_billboardType	= BillboardType::CAMERA_FACING_XY;
	SpriteSheet*	m_spriteSheet	= nullptr;
	SpriteAnimMap	m_animations;
};