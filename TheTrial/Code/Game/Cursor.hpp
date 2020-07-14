#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"

class Entity;
class SpriteSheet;

enum CursorState
{
	CURSOR_STATE_DEFAULT		= 0,
	CURSOR_STATE_ENEMY_HOVERED	= 1,
	CURSOR_STATE_MENU			= 2,
	CURSOR_STATE_ATTACK_MOVE	= 3,
};

class Cursor
{
public:
	Cursor();
	~Cursor();

	void Update();
	void Render() const;

	Vec2 const& GetWorldPosition() const	{ return m_worldPosition; }
	float		GetRadius() const			{ return m_radius; }
	CursorState GetState() const			{ return m_state; }

	void SetState( CursorState cursorState );
	void SetPosition( Vec2 const& position );

	bool IsOverlappingEntity( Entity* entity );

private:
	SpriteSheet* m_sheet	= nullptr;
	CursorState m_state		= CURSOR_STATE_DEFAULT;

	float m_radius			= 0.25;
	Rgba8 m_tint			= Rgba8::WHITE;
	AABB2 m_dimensions		= AABB2( -0.5f, -0.5f, 0.5f, 0.5f );
	Vec2 m_worldPosition	= Vec2( 0.0f, 0.0f );
	Vec2 m_uiPosition		= Vec2( 0.0f, 0.0f );
};