#include "Game/UIButton.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//---------------------------------------------------------------------------------------------------------
UIButton::UIButton( std::string const& text, Vec2 const& buttonDimensions, Vec2 const& buttonAlignment, Vec2 const& buttonOffset )
{
	m_textToDisplay = text;
	SetBounds( buttonDimensions, buttonAlignment, buttonOffset );
}


//---------------------------------------------------------------------------------------------------------
UIButton::~UIButton()
{
}


//---------------------------------------------------------------------------------------------------------
void UIButton::Update()
{
	if( m_buttonState != BUTTON_STATE_PRESSED && IsPointInsideAABB2D( g_theGame->GetCursorPosition(), m_bounds ) )
	{
		m_buttonState = BUTTON_STATE_HOVERED;
	}
	else if( m_buttonState != BUTTON_STATE_PRESSED )
	{
		m_buttonState = BUTTON_STATE_NORMAL;
	}

	if( m_buttonState == BUTTON_STATE_HOVERED && g_theInput->WasMouseButtonJustPressed( MOUSE_BUTTON_LEFT ) )
	{
		m_buttonState = BUTTON_STATE_PRESSED;
	}
	else if( m_buttonState == BUTTON_STATE_PRESSED && g_theInput->WasMouseButtonJustReleased( MOUSE_BUTTON_LEFT ) )
	{
		m_onClick.Invoke( m_parameters );
		m_buttonState = BUTTON_STATE_NORMAL;
	}

}


//---------------------------------------------------------------------------------------------------------
void UIButton::Render() const
{
	Rgba8 tintToRender = m_tint;
	if( m_buttonState == BUTTON_STATE_HOVERED )
	{
		tintToRender = m_hoveredTint;
	}
	else if( m_buttonState == BUTTON_STATE_PRESSED )
	{
		tintToRender = m_pressedTint;
	}

	//Draw ButtonBox
	std::vector<Vertex_PCU> buttonVerts;
	AppendVertsForAABB2D( buttonVerts, m_bounds, tintToRender, m_uvBox.mins, m_uvBox.maxes );

	g_theRenderer->BindTexture( m_backgroundTexture );
	g_theRenderer->BindShader( (Shader*)nullptr );
	g_theRenderer->DrawVertexArray( buttonVerts );


	//DrawButtonText
	if( m_textToDisplay.size() > 0 )
	{
		std::vector<Vertex_PCU> buttonTextVerts;
		g_devConsoleFont->AddVertsForTextInBox2D( buttonTextVerts, m_bounds, m_textSize, m_textToDisplay, m_textTint, 1.f, m_textAlignment );

		g_theRenderer->BindShader( nullptr );
		g_theRenderer->BindTexture( g_devConsoleFont->GetTexture() );
		g_theRenderer->DrawVertexArray( buttonTextVerts );
	}
}


//---------------------------------------------------------------------------------------------------------
void UIButton::SetBounds( Vec2 const& dimensions, Vec2 const& alignment, Vec2 const& offset )
{
	m_dimensions = dimensions;
	m_alignment = alignment;
	m_offset = offset;
	UpdateBounds();
}


//---------------------------------------------------------------------------------------------------------
void UIButton::SetDimensions( Vec2 const& dimensions )
{
	m_dimensions = dimensions;
	UpdateBounds();
}


//---------------------------------------------------------------------------------------------------------
void UIButton::SetButtonAlignment( Vec2 const& buttonAlignment )
{
	m_alignment = buttonAlignment;
	UpdateBounds();
}


//---------------------------------------------------------------------------------------------------------
void UIButton::SetOffset( Vec2 const& offset )
{
	m_offset = offset;
	UpdateBounds();
}

//---------------------------------------------------------------------------------------------------------
void UIButton::SetTextToDisplay( std::string const& text )
{
	m_textToDisplay = text;
}


//---------------------------------------------------------------------------------------------------------
void UIButton::SetTextSize( float textSize )
{
	m_textSize = textSize;
}


//---------------------------------------------------------------------------------------------------------
void UIButton::SetTextAlignment( Vec2 const& textAlignment )
{
	m_textAlignment = textAlignment;
}


//---------------------------------------------------------------------------------------------------------
void UIButton::SetTextTint( Rgba8 const& textTint )
{
	m_textTint = textTint;
}


//---------------------------------------------------------------------------------------------------------
void UIButton::SetButtonTint( Rgba8 const& buttonTint )
{
	m_tint = buttonTint;
}


//---------------------------------------------------------------------------------------------------------
void UIButton::SetHoveredTint( Rgba8 const& hoveredTint )
{
	m_hoveredTint = hoveredTint;
}


//---------------------------------------------------------------------------------------------------------
void UIButton::SetPressedTint( Rgba8 const& pressedTint )
{
	m_pressedTint = pressedTint;
}


//---------------------------------------------------------------------------------------------------------
void UIButton::SetButtonTexture( Texture* backgroundTexture, Vec2 const& textureMinUVs, Vec2 const& textureMaxUVs )
{
	m_backgroundTexture = backgroundTexture;
	m_uvBox.mins = textureMinUVs;
	m_uvBox.maxes = textureMaxUVs;
}


//---------------------------------------------------------------------------------------------------------
void UIButton::SetButtonUVs(Vec2 const& textureMinUVs, Vec2 const& textureMaxUVs )
{
	m_uvBox.mins = textureMinUVs;
	m_uvBox.maxes = textureMaxUVs;
}


//---------------------------------------------------------------------------------------------------------
void UIButton::UpdateBounds()
{
	Camera* worldCamera		= g_theGame->GetPlayerCamera();
	Vec3	orthoBottomLeft = worldCamera->GetOrthoBottomLeft();
	Vec3	orthoTopRight	= worldCamera->GetOrthoTopRight();

	AABB2	screenRect	= AABB2( orthoBottomLeft.x, orthoBottomLeft.y, orthoTopRight.x, orthoTopRight.y );
	m_bounds			= screenRect.GetBoxWithin( m_dimensions, m_alignment );
	m_bounds.Translate( m_offset );
}
