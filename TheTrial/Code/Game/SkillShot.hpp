#pragma once
#include "Game/Ability.hpp"
#include "Game/Item.hpp"


class SkillShot : public Ability
{
public:
	SkillShot() = default;
	SkillShot( XmlElement const& xmlElement );
	SkillShot( SkillShot const& copyFrom );
	~SkillShot();

	virtual void Use() override;

private:
	float m_range = 1.f;
	float m_projectileSpeed = 1.f;
	float m_aimAperatureDegrees = 90.f;

	int m_numProjectiles = 1;
	int m_baseDamagePerProjectile = 5;

	ActorStat m_statToScale = STAT_ATTACK_DAMAGE;
	float m_scaleFraction = 0.f;
};