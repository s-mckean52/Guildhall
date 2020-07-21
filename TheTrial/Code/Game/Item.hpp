#pragma once
#include "Game/Entity.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include <string>
#include <vector>
#include <map>

class SpriteSheet;
class SpriteDefinition;

//---------------------------------------------------------------------------------------------------------
enum ActorStat
{
	STAT_CRIT_MULTIPLIER,
	STAT_CRIT_CHANCE,
	STAT_MOVEMENT_SPEED,
	STAT_ATTACK_SPEED,
	STAT_ATTACK_DAMAGE,
};

//---------------------------------------------------------------------------------------------------------
struct StatMod
{
public:
	ActorStat statToMod = STAT_ATTACK_DAMAGE;
	float amountToAdd = 0.f;

public:
	StatMod() = default;
	StatMod( ActorStat newStatToMod, float newAmountToAdd );
};


//---------------------------------------------------------------------------------------------------------
class Item : public Entity
{
public:
	Item();
	explicit Item( XmlElement const& element );
	explicit Item( Item const& copyFrom );
	~Item();

	void Render() const					override;
	void Update( float deltaSeconds )	override;

	void DrawItemStats() const;

	std::string		GetName() const			{ return m_name; }
	size_t			GetNumStatMods() const	{ return m_statMods.size(); }
	StatMod const&	GetStatModAtIndex( int index ) const;

public:
	static void			CreateItemsFromXML( const char* filepath );
	static Item const&	GetItemDefByName( std::string const& itemName );
	static ActorStat	GetStatTypeFromString( std::string const& statTypeAsString );
	static std::string	GetStringForActorStat( ActorStat actorStat );
	static Item*		GetRandomItem();

private:
	static void CreateSpriteSheet( XmlElement const& element );
	static void CreateItem( XmlElement const& element );

private:
	static std::map<std::string, SpriteSheet*>	s_itemSpriteSheets;
	static std::map<std::string, Item>			s_itemDefinitions;

private:
	bool m_isHovered = false;

	AABB2					m_spriteUVBounds;
	std::string				m_name				= "";
	Rgba8					m_tint				= Rgba8::WHITE;
	std::vector< StatMod >	m_statMods;
};