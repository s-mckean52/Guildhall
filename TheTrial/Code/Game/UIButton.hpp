#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Delegate.hpp"
#include "Engine/Core/EventSubscription.hpp"
#include <string>

enum ButtonState
{
	BUTTON_STATE_NORMAL,
	BUTTON_STATE_HOVERED,
	BUTTON_STATE_PRESSED,
};

struct	Texture;

class UIButton
{
public:
	UIButton(	std::string const& text,
				Vec2 const& buttonDimensions	= Vec2::UNIT,
				Vec2 const& buttonAlignment		= ALIGN_CENTERED,
				Vec2 const& buttonOffset		= Vec2::ZERO );
	~UIButton();

	void Update();
	void Render() const;

	//Accessors
	AABB2 GetBounds() const { return m_bounds; }

	//Mutators
	void SetBounds( Vec2 const& dimensions, Vec2 const& alignment, Vec2 const& offset );
	void SetDimensions( Vec2 const& dimensions );
	void SetButtonAlignment( Vec2 const& buttonAlignment );
	void SetOffset( Vec2 const& offset );
	void SetTextToDisplay( std::string const& text );
	void SetTextSize( float textSize );
	void SetTextAlignment( Vec2 const& textAlignment );
	void SetTextTint( Rgba8 const& textTint );
	void SetButtonTint( Rgba8 const& buttonTint );
	void SetHoveredTint( Rgba8 const& hoveredTint );
	void SetPressedTint( Rgba8 const& pressedTint );
	void SetButtonTexture( Texture* backgroundTexture, Vec2 const& textureMinUVs = Vec2::ZERO, Vec2 const& textureMaxUVs = Vec2::UNIT );
	void SetButtonUVs( Vec2 const& textureMinUVs = Vec2::ZERO, Vec2 const& textureMaxUVs = Vec2::UNIT );

	template<typename OBJ_TYPE>
	void SetOnClick( OBJ_TYPE* obj, void( OBJ_TYPE::*methodCallback )( NamedProperties* parameters ), NamedProperties* parametersToUse = nullptr );

public:
	void UpdateBounds();

private:
	AABB2		m_bounds;
	Vec2		m_dimensions	= Vec2::UNIT;
	Vec2		m_alignment		= ALIGN_CENTERED;
	Vec2		m_offset		= Vec2::ZERO;

	std::string m_textToDisplay = "";
	float		m_textSize		= 1.f;
	Vec2		m_textAlignment = ALIGN_CENTERED;
	Rgba8		m_textTint		= Rgba8::BLACK;

	ButtonState m_buttonState	= BUTTON_STATE_NORMAL;
	Rgba8 m_tint				= Rgba8::WHITE;
	Rgba8 m_hoveredTint			= Rgba8::GREEN;
	Rgba8 m_pressedTint			= Rgba8::BLUE;

	AABB2 m_uvBox;
	Texture* m_backgroundTexture = nullptr;

	Delegate<NamedProperties*> m_onClick;
	NamedProperties* m_parameters;
};


//---------------------------------------------------------------------------------------------------------
template<typename OBJ_TYPE>
void UIButton::SetOnClick( OBJ_TYPE* obj, void( OBJ_TYPE::* methodCallback )( NamedProperties* parametersToUse ), NamedProperties* parametersToUse )
{
	m_onClick.SubscribeMethod( obj, methodCallback );
	m_parameters = parametersToUse;
}
