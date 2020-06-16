#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Game/Ability.hpp"
#include <vector>

class Buff : public Ability
{
public:
	Buff() = default;
	Buff( XmlElement const& xmlElement );
	Buff( Buff const& copyFrom );
	~Buff();

	virtual void Use() override;

private:
	Rgba8 m_tint		= Rgba8::WHITE;
	float m_duration	= 0.f;

	std::vector<StatMod> m_statMods;
};