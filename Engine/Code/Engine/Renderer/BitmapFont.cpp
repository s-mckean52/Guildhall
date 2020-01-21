#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/AABB2.hpp"


//---------------------------------------------------------------------------------------------------------
BitmapFont::BitmapFont( const char* fontName, const Texture* fontTexture )
	: m_fontName( fontName )
	, m_glyphSpriteSheet( *fontTexture, IntVec2( 16, 16 ) )
{
}


//---------------------------------------------------------------------------------------------------------
const Texture* BitmapFont::GetTexture() const
{
	return &m_glyphSpriteSheet.GetTexture();
}


//---------------------------------------------------------------------------------------------------------
void BitmapFont::AddVertsForText2D( std::vector<Vertex_PCU>& vertexArray, const Vec2& textMins, float cellHeight, const std::string& text, const Rgba8& tint, float cellAspect )
{
	Vec2 glyphMins;
	Vec2 glyphMaxes( 0.f, cellHeight );

	Vec2 uvsAtMins;
	Vec2 uvsAtMaxes;

	for( int stringCharIndex = 0; stringCharIndex < text.length(); ++stringCharIndex )
	{
		m_glyphSpriteSheet.GetSpriteUVs( uvsAtMins, uvsAtMaxes, text[ stringCharIndex ] );

		glyphMaxes.x += cellHeight * GetGlyphAspect( text[ stringCharIndex ] ) * cellAspect;

		AppendVertsForAABB2D( vertexArray, AABB2( glyphMins, glyphMaxes ), tint, uvsAtMins, uvsAtMaxes );

		glyphMins = Vec2( glyphMaxes.x, glyphMins.y );
	}

	TransformVertexArray( vertexArray, 1.f, 0.f, textMins );
}


//---------------------------------------------------------------------------------------------------------
float BitmapFont::GetGlyphAspect( int glyphUnicode ) const
{
	SpriteDefinition glyphDefinition = m_glyphSpriteSheet.GetSpriteDefinition( glyphUnicode );
	return glyphDefinition.GetAspect();
}


//---------------------------------------------------------------------------------------------------------
void BitmapFont::AddVertsForTextInBox2D( std::vector<Vertex_PCU>& vertexArray, const AABB2& box, float cellHeight, const std::string& text, const Rgba8& tint, float cellAspect, const Vec2& alignment )
{
	Vec2 textDimensions = GetDimensionsForText2D( cellHeight, text, cellAspect );
	Vec2 boxDimensions	= box.GetDimensions();

	Vec2 textMins	= box.mins;
	textMins		+= ( boxDimensions - textDimensions ) * alignment;

	AddVertsForText2D( vertexArray, textMins, cellHeight, text, tint, cellAspect );
}


//---------------------------------------------------------------------------------------------------------
Vec2 BitmapFont::GetDimensionsForText2D( float cellHeight, const std::string& text, float cellAspect )
{
	float stringWidth = 0.f;
	size_t stringLength = text.length();

	for( int stringIndex = 0; stringIndex < stringLength; ++stringIndex )
	{
		stringWidth += cellHeight * GetGlyphAspect( text[ stringIndex ] ) * cellAspect;
	}

	return Vec2( stringWidth, cellHeight );
}

