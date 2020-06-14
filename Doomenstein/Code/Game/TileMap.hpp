#pragma once
#include "Game/Map.hpp"
#include "Game/Entity.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/XmlUtils.hpp"

class Tile;
class Camera;

class TileMap : public Map
{
public:
	TileMap( Game* theGame, World* theWorld, XmlElement const& xmlElement );
	~TileMap();

	void Update() override;
	void Render() const override;

	bool	IsTileSolid( Tile* tile ) const;
	IntVec2 GetTileXYCoordsForTileIndex( int tileIndex ) const;
	Vec3	GetTilePositionForTileIndex( int tileIndex ) const;
	int		GetTileIndexFromCoords( IntVec2 tileCoords ) const;
	Tile*	GetTileByCoords( IntVec2 tileCoords ) const;
	Tile*	GetTileInDirectionFromTileIndex( int tileIndex, int relativeXDistance, int relativeYDistance ) const;

	void	CreatePlayerStart( XmlElement const& xmlElement );

	void CreateMapVerts();
	void AppendVertsForTile( int tileIndex );
	void AppendVertsForOpenTile( int tileIndex );
	void AppendVertsForSolidTile( int tileIndex );

	void SpawnEntity( EntityType entityType );

private:
	void CreateFromXML( XmlElement const& xmlElement );
	
	void CreateLegendFromXML( XmlElement const& xmlElement );
	void AddGlyphToLegend( XmlElement const& xmlElement );
	void AddTileByLegendGlyph( char glyph, int xPosition, int yPosition );
	
	void CreateTilesFromXML( XmlElement const& xmlElement );
	void CreateEntitiesFromXML( XmlElement const& xmlElement );

private:
	IntVec2	m_dimensions = IntVec2( -1, -1 );
	
	std::vector<Tile*>			m_tiles;
	std::vector<Vertex_PCUTBN>	m_mapVerts;

	std::map<char, std::string> m_legend;
};