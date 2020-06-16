#pragma once
#include "Game/Ability.hpp"


class Blink : public Ability
{
public:
	Blink() = default;
	Blink( XmlElement const& xmlElement );
	Blink( Game* theGame, Actor* owner, float range );
	Blink( Blink const& copyFrom );
	~Blink();

	virtual void Use() override;

private:
	float m_range = 0.f;
};