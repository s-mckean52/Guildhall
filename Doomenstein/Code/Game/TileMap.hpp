#pragma once
#include "Game/Map.hpp"
#include "Game/EntityDef.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/XmlUtils.hpp"

class Tile;
class Camera;

class TileMap : public Map
{
public:
	TileMap( Game* theGame, World* theWorld, std::string const& name, XmlElement const& xmlElement );
	~TileMap();

	RaycastResult		Raycast( Vec3 const& startPosition, Vec3 const& fwdDir, float maxDistance ) override;
	RaycastResult		RaycastAgainstCeilingAndFloor( Vec3 const& startPosition, Vec3 const& fwdDir, float maxDistance );
	RaycastResult		RaycastAgainstWalls( Vec3 const& startPosition, Vec3 const& fwdDir, float maxDistance );
	RaycastResult		RaycastAgainstEntities( Vec3 const& startPosition, Vec3 const& fwdDir, float maxDistance );
	RaycastResult		GetBestRaycast( std::vector<RaycastResult> const& results );
	std::vector<Vec3>	GetRayImpactPointsSideView( Entity* entity, std::vector<Vec3> const& potentialHits );
	Vec3				GetClosestPointFromList( Vec3 const& point, std::vector<Vec3> const& pointsToCheck );


	void Update() override;
	void UpdateEntities();

	void Render() const override;
	void RenderMap() const;
	void RenderEntities() const;

	bool	IsTileSolid( Tile* tile ) const;
	IntVec2 GetTileXYCoordsForTileIndex( int tileIndex ) const;
	Vec3	GetTilePositionForTileIndex( int tileIndex ) const;
	int		GetTileIndexFromCoords( IntVec2 tileCoords ) const;
	Tile*	GetTileByCoords( IntVec2 tileCoords ) const;
	Tile*	GetTileInDirectionFromTileIndex( int tileIndex, int relativeXDistance, int relativeYDistance ) const;
	AABB2	GetTileXYBounds( Tile* tile ) const;

	void	CreatePlayerStart( XmlElement const& xmlElement );

	void CreateMapVerts();
	void AppendVertsForTile( int tileIndex );
	void AppendVertsForOpenTile( int tileIndex );
	void AppendVertsForSolidTile( int tileIndex );

private:
	void CreateFromXML( XmlElement const& xmlElement );
	
	void CreateLegendFromXML( XmlElement const& xmlElement );
	void AddGlyphToLegend( XmlElement const& xmlElement );
	void AddTileByLegendGlyph( char glyph, int xPosition, int yPosition );
	
	void CreateTilesFromXML( XmlElement const& xmlElement );
	void CreateEntitiesFromXML( XmlElement const& xmlElement );

	void HandleEntitiesVWallCollisions();
	void HandleEntityVWallCollisions( Entity* entity );
	void PushEntityOutOfWall( Entity* entity, int xDir, int yDir );

private:
	IntVec2	m_dimensions = IntVec2( -1, -1 );
	
	std::vector<Tile*>			m_tiles;
	std::vector<Vertex_PCUTBN>	m_mapVerts;

	std::map<char, std::string> m_legend;
};