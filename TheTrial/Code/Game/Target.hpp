#pragma once
#include "Game/Ability.hpp"
#include "Game/Item.hpp"


class Target : public Ability
{
public:
	Target() = default;
	Target( XmlElement const& xmlElement );
	Target( Target const& copyFrom );
	~Target();

	virtual void Use() override;

private:
	float m_duration = 0.f;
	int m_numTicks = 1;
	float m_damagePerTick = 0.f;
	float m_range = 0.f;

	std::vector<StatMod> m_statMods;
};