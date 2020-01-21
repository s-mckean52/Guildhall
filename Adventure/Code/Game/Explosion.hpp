#pragma once
#include "Game/Entity.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

class Explosion : public Entity
{
public:
	~Explosion();
	explicit Explosion( Map* theMap, const Vec2& startPosition, float duration = 2.f, float radius = 0.1f);

	virtual void Update( float seconds ) override;
	virtual void Render() const override;

public:
	static void CreateExplosionSpriteSheet();

public:
	static Texture* s_explosionTexture;
	static SpriteSheet* s_explosionSpriteSheet;

private:
	SpriteAnimDefinition* m_explosionAnim = nullptr;
	float m_duration = 2.f;
	float m_timeElapsed = 0.f;
};
