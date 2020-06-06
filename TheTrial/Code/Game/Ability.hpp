#pragma once
#include <string>

class Timer;
class Game;
class Entity;

class Ability
{
public:
	Ability( Game* theGame, Entity* owner, double cooldownSeconds, std::string const& name );
	~Ability();

	virtual void Update();
	virtual void Render() const;
	
	virtual void Use();

	bool IsOnCooldown() const;

protected:
	Game* m_theGame = nullptr;
	Entity* m_owner = nullptr;
	Timer* m_cooldownTimer = nullptr;
	std::string m_name = "Default Name";
	double m_baseCooldownSeconds = 0.0;
};
