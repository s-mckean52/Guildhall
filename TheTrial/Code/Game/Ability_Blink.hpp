#pragma once
#include "Game/Ability.hpp"


class Blink : public Ability
{
public:
	Blink( Game* theGame, Entity* owner );
	~Blink();

	virtual void Use() override;
};