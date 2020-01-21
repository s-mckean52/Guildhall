#pragma once
#include "Game/Entity.hpp"
#include <vector>

struct Vertex_PCU;
struct Texture;
class SpriteSheet;

class Boulder : public Entity
{
public:
	Boulder( Map* theMap, Vec2 position, EntityFaction faction );
	~Boulder() {};

	void Render() const override;
	void Update( float deltaSeconds ) override;
	void DebugDraw() const override;
	
private:
	void CreateBoulderSpriteSheet();

private:
	Texture* m_texture_4x4;
	SpriteSheet* m_spriteSheet;

	Vec2 m_uvAtMins;
	Vec2 m_uvAtMaxes;
};