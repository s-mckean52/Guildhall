#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include <vector>

class TileDefinition;
struct AABB2;
struct Rgba8;

struct Tile
{
public:
	IntVec2 m_tileCoords;
	TileDefinition* m_tileDef = nullptr;

public:
	//Constructors
	explicit Tile( TileDefinition* tileDef, const IntVec2& tileCoords );

	//Accessors
	IntVec2				GetCoords() const				{ return m_tileCoords; }
	TileDefinition*		GetTileDefinition() const		{ return m_tileDef; }
	AABB2				GetWorldBounds() const;
	Vec2				GetCenterPosition() const;

	//Mutators
	void				SetTileDefinition( TileDefinition* tileDef );
	void				SetTileDefinitionWithTexel( const Rgba8& texelColor );
	
	//---------------------------------------------------------------------------------------------------------
	void AppendVertsForRender( std::vector<Vertex_PCU>& vertexArray ) const;
};