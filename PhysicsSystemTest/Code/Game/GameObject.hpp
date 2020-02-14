#pragma once
#include "Engine/Core/Rgba8.hpp"

class Rigidbody2D;
struct Vec2;

class GameObject
{
public:
	GameObject();
	~GameObject();

	void Update( float deltaSeconds );
	void UpdateColors();
	void UpdateColorBasedOnPhysics();

	void Draw() const;

	void SetPosition( Vec2 position );

public:
	bool m_isHovered		= false;
	bool m_isHeld			= false;
	bool m_isOverlapping	= false;

	bool m_isDestroyed		= false;

	Rigidbody2D* m_rigidbody = nullptr;
	Rgba8 m_defaultBorderColor;
	Rgba8 m_startFillColor;
	Rgba8 m_currentBorderColor;
	Rgba8 m_currentFillColor;
};