#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include <vector>

class	TileDefinition;
struct	Vertex_PCU;
struct	Rgba8; 

struct Tile
{
private:
	IntVec2 m_tileCoords;
	TileDefinition* m_tileDef = nullptr;

public:
	//Constructors
	explicit Tile( TileDefinition* tileDef, const IntVec2& tileCoords );

	//Accessors
	IntVec2				GetCoords() const;
	TileDefinition*		GetTileDefinition() const;
	AABB2				GetWorldBounds() const;

	//Mutators
	void				SetTileDefinition( TileDefinition* tileDef );
	void				SetTileDefinitionWithTexel( const Rgba8& texelColor );
	
	//---------------------------------------------------------------------------------------------------------
	void AppendVertsForRender( std::vector<Vertex_PCU>& vertexArray ) const;
};