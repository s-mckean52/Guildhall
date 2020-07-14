#pragma once
#include "Game/Actor.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Timer.hpp"
#include <string>
#include <map>

class Game;
struct Vec2;

//---------------------------------------------------------------------------------------------------------
enum AbilityType
{
	ABILITY_TYPE_BLINK,
	ABILITY_TYPE_SKILLSHOT,
	ABILITY_TYPE_TARGET,
	ABILITY_TYPE_BUFF,

	NUM_ABILITY_TYPES,
	
	INVALID_ABILITY_TYPE
};


//---------------------------------------------------------------------------------------------------------
class Ability
{
public:
	Ability() = default;
	Ability( XmlElement const& xmlElement );
	Ability( Game* theGame, Actor* owner, double cooldownSeconds, std::string const& name );
	Ability( Ability const& copyFrom );
	virtual ~Ability();

	virtual void Update();
	virtual void Render( Vec2 const& minPosition, char abilityKey ) const;
	virtual void Use();

	bool IsOnCooldown() const;
	
	void SetGame( Game* theGame );
	void SetOwner( Actor* owner );

public:
	static void			CreateAbilitiesFromXML( char const* filepath );
	static AbilityType	GetAbilityTypeFromString( std::string const& abilityTypeAsString );
	static std::string	GetAbilityTypeAsString( AbilityType abilityType );
	static Ability*		GetNewAbilityByName( std::string const& abilityName );
	static Strings		GetAbilityList( int startIndex, int endIndex );
	static int			GetNumAbilities();

private:
	static std::map<std::string, Ability*> s_abilities;

protected:
	Game* m_theGame = nullptr;
	Actor* m_owner = nullptr;

	AbilityType	m_type					= INVALID_ABILITY_TYPE;
	std::string	m_name					= "Default Name";
	double		m_baseCooldownSeconds	= 0.0;
	Timer		m_cooldownTimer;
};
