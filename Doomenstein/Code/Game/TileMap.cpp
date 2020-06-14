#include "Game/TileMap.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapRegion.hpp"
#include "Game/MapMaterial.hpp"
#include "Game/Tile.hpp"
#include "Game/PlayerStart.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/DevConsole.hpp"


//---------------------------------------------------------------------------------------------------------
TileMap::TileMap( Game* theGame, World* theWorld, XmlElement const& xmlElement )
	: Map( theGame, theWorld )
{
	m_mapMesh = new GPUMesh( g_theRenderer );
	CreateFromXML( xmlElement );
}


//---------------------------------------------------------------------------------------------------------
TileMap::~TileMap()
{
}


//---------------------------------------------------------------------------------------------------------
void TileMap::Update()
{
	CreateMapVerts();
}

//---------------------------------------------------------------------------------------------------------
void TileMap::Render() const
{
	g_theRenderer->BindTextureByPath( "Data/Images/Terrain_8x8.png" );
	g_theRenderer->BindShader( (Shader*)nullptr );

	g_theRenderer->DrawMesh( m_mapMesh );
}


//---------------------------------------------------------------------------------------------------------
IntVec2 TileMap::GetTileXYCoordsForTileIndex( int tileIndex ) const
{
	int yPos = tileIndex / m_dimensions.x;
	int xPos = tileIndex - ( yPos * m_dimensions.x );
	return IntVec2( xPos, yPos );
}


//---------------------------------------------------------------------------------------------------------
Vec3 TileMap::GetTilePositionForTileIndex( int tileIndex ) const
{
	IntVec2 tileCoords = m_tiles[ tileIndex ]->GetTileCoords();
	return Vec3( static_cast<float>( tileCoords.x ), static_cast<float>( tileCoords.y ), 0.f );
}


//---------------------------------------------------------------------------------------------------------
int TileMap::GetTileIndexFromCoords( IntVec2 tileCoords ) const
{
	int tileIndex = ( tileCoords.y * m_dimensions.x ) + tileCoords.x;
	return tileIndex;
}


//---------------------------------------------------------------------------------------------------------
Tile* TileMap::GetTileByCoords( IntVec2 tileCoords ) const
{
	int tileIndex = GetTileIndexFromCoords( tileCoords );
	return m_tiles[ tileIndex ];
}


//---------------------------------------------------------------------------------------------------------
void TileMap::CreateLegendFromXML( XmlElement const& xmlElement )
{
	XmlElement const* nextChildElement = xmlElement.FirstChildElement();
	for( ;; )
	{
		if( nextChildElement == nullptr )
			break;

		std::string elementName = nextChildElement->Name();
		if( elementName == "Tile" )
		{
			AddGlyphToLegend( *nextChildElement );
		}
		nextChildElement = nextChildElement->NextSiblingElement();
	}
}


//---------------------------------------------------------------------------------------------------------
void TileMap::CreateTilesFromXML( XmlElement const& xmlElement )
{
	int yPos = m_dimensions.y - 1;

	XmlElement const* nextChildElement = xmlElement.FirstChildElement();
	for( ;; )
	{
		if( nextChildElement == nullptr )
			break;

		std::string mapRow = ParseXmlAttribute( *nextChildElement, "tiles", "MISSING" );
		for( int glyphIndex = 0; glyphIndex < mapRow.length(); ++glyphIndex )
		{
			char glyph = mapRow[ glyphIndex ];
			AddTileByLegendGlyph( glyph, glyphIndex, yPos );
		}
		nextChildElement = nextChildElement->NextSiblingElement();
		--yPos;
	}
}


//---------------------------------------------------------------------------------------------------------
void TileMap::CreateEntitiesFromXML( XmlElement const& xmlElement )
{
	XmlElement const* nextChildElement = xmlElement.FirstChildElement();
	for( ;; )
	{
		if( nextChildElement == nullptr )
			break;

		std::string entityTypeAsString = nextChildElement->Name();
		EntityType entityTypeToSpawn = Entity::GetEntityTypeFromString( entityTypeAsString );
		
		switch( entityTypeToSpawn )
		{
		case ENTITY_PLAYER_START:
			CreatePlayerStart( *nextChildElement );
			break;
		case INVALID_ENTITY_TYPE:
			g_theConsole->ErrorString( "%s is an unsupported entity type", entityTypeAsString.c_str() );
			break;
		default:
			g_theConsole->ErrorString( "Entity Spawning is currently unsupported" );
			//TODO: SpawnEntity( entityTypeToSpawn );
			break;
		}

		nextChildElement = nextChildElement->NextSiblingElement();
	}
}


//---------------------------------------------------------------------------------------------------------
void TileMap::AddGlyphToLegend( XmlElement const& xmlElement )
{
	char tileGlyph				= ParseXmlAttribute( xmlElement, "glyph", '`' );
	std::string regionTypeName	= ParseXmlAttribute( xmlElement, "regionType", "MISSING" );
	
	m_legend[ tileGlyph ] = regionTypeName;
}


//---------------------------------------------------------------------------------------------------------
void TileMap::AddTileByLegendGlyph( char glyph, int xPosition, int yPosition )
{
	std::string regionTypeName = m_legend[ glyph ];
	MapRegion* regionType = MapRegion::GetRegionByName( regionTypeName );
	int tileIndex = ( yPosition * m_dimensions.x ) + xPosition;
	m_tiles[ tileIndex ] = new Tile( regionType, IntVec2( xPosition, yPosition ) );
}


//---------------------------------------------------------------------------------------------------------
Tile* TileMap::GetTileInDirectionFromTileIndex( int tileIndex, int relativeXDistance, int relativeYDistance ) const
{
	IntVec2 currentTileCoords = GetTileXYCoordsForTileIndex( tileIndex );
	IntVec2 tileCoordsToGet = currentTileCoords + IntVec2( relativeXDistance, relativeYDistance );

	if( tileCoordsToGet.x < 0 || tileCoordsToGet.x >= m_dimensions.x )
	{
		return nullptr;
	}
	else if( tileCoordsToGet.y < 0 || tileCoordsToGet.y >= m_dimensions.y )
	{
		return nullptr;
	}
	else
	{
		return GetTileByCoords( tileCoordsToGet );
	}
}


//---------------------------------------------------------------------------------------------------------
void TileMap::CreatePlayerStart( XmlElement const& xmlElement )
{
	Vec2 invalidPlayerStartPosition = Vec2( -1.f, -1.f );
	Vec2 playerStartPositionXY = ParseXmlAttribute( xmlElement, "pos", invalidPlayerStartPosition );
	if( playerStartPositionXY == invalidPlayerStartPosition )
	{
		playerStartPositionXY = Vec2( 1.f, 1.f );
		g_theConsole->ErrorString( "Could not find position for Player Start - Position set to (1,1)" );
	}
	float invalidYaw = ~0;
	float playerStartYawDegrees = ParseXmlAttribute( xmlElement, "yaw", invalidYaw );
	if( playerStartYawDegrees == invalidYaw )
	{
		playerStartYawDegrees = 0.f;
		g_theConsole->ErrorString( "Could not find yaw for Player Start - Yaw set to 0 Degrees" );
	}
	m_playerStart = new PlayerStart( m_game, m_world, this, playerStartPositionXY, playerStartYawDegrees );
}


//---------------------------------------------------------------------------------------------------------
void TileMap::CreateMapVerts()
{
	size_t numTiles = m_dimensions.x * m_dimensions.y;
	size_t numFaces = numTiles * 4;
	size_t numVerticies = numFaces * 6;
	m_mapVerts.clear();
	m_mapVerts.reserve( numVerticies );


	for( uint tileIndex = 0; tileIndex < numTiles; ++tileIndex )
	{
		AppendVertsForTile( tileIndex );
 	}

	m_mapMesh->UpdateVerticies( static_cast<uint>( m_mapVerts.size() ), &m_mapVerts[0] );
}


//---------------------------------------------------------------------------------------------------------
void TileMap::AppendVertsForTile( int tileIndex )
{
	Tile* tileToAppend = m_tiles[ tileIndex ];
	if( IsTileSolid( tileToAppend ) )
	{
		AppendVertsForSolidTile( tileIndex );
	}
	else if( !IsTileSolid( tileToAppend ) )
	{
		AppendVertsForOpenTile( tileIndex );
	}
}


//---------------------------------------------------------------------------------------------------------
void TileMap::AppendVertsForOpenTile( int tileIndex )
{
	Rgba8 color = Rgba8::WHITE;

	Vec3 minTilePosition	= GetTilePositionForTileIndex( tileIndex );
	Vec3 frontBottomLeft	= minTilePosition;
	Vec3 frontBottomRight	= frontBottomLeft + Vec3::UNIT_POSITIVE_X;
	Vec3 frontTopLeft		= frontBottomLeft + Vec3::UNIT_POSITIVE_Z;
	Vec3 frontTopRight		= frontBottomLeft + Vec3( 1.f, 0.f, 1.f );

	//looking from front Back
	Vec3 backBottomLeft		= minTilePosition + Vec3::UNIT_POSITIVE_Y;
	Vec3 backBottomRight	= backBottomLeft + Vec3::UNIT_POSITIVE_X;
	Vec3 backTopLeft		= backBottomLeft + Vec3::UNIT_POSITIVE_Z;
	Vec3 backTopRight		= backBottomLeft + Vec3( 1.f, 0.f, 1.f );

	Vec3 tangent		= Vec3::UNIT_POSITIVE_X;
	Vec3 bitangent		= Vec3::UNIT_POSITIVE_Y;
	Vec3 topNormal		= Vec3::UNIT_NEGATIVE_Z;
	Vec3 bottomNormal	= Vec3::UNIT_POSITIVE_Z;

	MapRegion* tileRegionType = m_tiles[ tileIndex ]->GetRegionType();

	MapMaterial* ceilingMaterial = tileRegionType->GetCeilingMaterial();
	AABB2 ceilingUVBox = ceilingMaterial->GetUVBox();
	Vec2 ceilingTopLeftUV		= Vec2( ceilingUVBox.mins.x, ceilingUVBox.maxes.y );
	Vec2 ceilingBottomRightUV	= Vec2( ceilingUVBox.maxes.x, ceilingUVBox.mins.y );
	
	MapMaterial* floorMaterial = tileRegionType->GetFloorMaterial();
	AABB2 floorUVBox = floorMaterial->GetUVBox();
	Vec2 floorTopLeftUV		= Vec2( floorUVBox.mins.x, floorUVBox.maxes.y );
	Vec2 floorBottomRightUV	= Vec2( floorUVBox.maxes.x, floorUVBox.mins.y );

	//Top																									 		      
	m_mapVerts.push_back(	Vertex_PCUTBN( frontTopRight,		color,	tangent,	bitangent,		topNormal,		ceilingUVBox.mins		) );
	m_mapVerts.push_back(	Vertex_PCUTBN( frontTopLeft,		color,	tangent,	bitangent,		topNormal,		ceilingBottomRightUV	) );
	m_mapVerts.push_back(	Vertex_PCUTBN( backTopLeft,			color,	tangent,	bitangent,		topNormal,		ceilingUVBox.maxes		) );

	m_mapVerts.push_back(	Vertex_PCUTBN( frontTopRight,		color,	tangent,	bitangent,		topNormal,		ceilingUVBox.mins		) );
	m_mapVerts.push_back(	Vertex_PCUTBN( backTopLeft,			color,	tangent,	bitangent,		topNormal,		ceilingUVBox.maxes		) );
	m_mapVerts.push_back(	Vertex_PCUTBN( backTopRight, 		color,	tangent,	bitangent,		topNormal,		ceilingTopLeftUV		) );

	//Bottom
	m_mapVerts.push_back(	Vertex_PCUTBN( frontBottomLeft,		color,	tangent,	bitangent,		topNormal,		floorUVBox.mins			) );
	m_mapVerts.push_back(	Vertex_PCUTBN( frontBottomRight,	color,	tangent,	bitangent,		topNormal,		floorBottomRightUV		) );
	m_mapVerts.push_back(	Vertex_PCUTBN( backBottomRight,		color,	tangent,	bitangent,		topNormal,		floorUVBox.maxes		) );

	m_mapVerts.push_back(	Vertex_PCUTBN( frontBottomLeft,		color,	tangent,	bitangent,		topNormal,		floorUVBox.mins			) );
	m_mapVerts.push_back(	Vertex_PCUTBN( backBottomRight,		color,	tangent,	bitangent,		topNormal,		floorUVBox.maxes		) );
	m_mapVerts.push_back(	Vertex_PCUTBN( backBottomLeft,		color,	tangent,	bitangent,		topNormal,		floorTopLeftUV			) );
}


//---------------------------------------------------------------------------------------------------------
void TileMap::AppendVertsForSolidTile( int tileIndex )
{
	Rgba8 color = Rgba8::WHITE;

	Vec3 minTilePosition	= GetTilePositionForTileIndex( tileIndex );
	Vec3 frontBottomLeft	= minTilePosition;
	Vec3 frontBottomRight	= frontBottomLeft + Vec3::UNIT_POSITIVE_X;
	Vec3 frontTopLeft		= frontBottomLeft + Vec3::UNIT_POSITIVE_Z;
	Vec3 frontTopRight		= frontBottomLeft + Vec3( 1.f, 0.f, 1.f );

	//looking from front Back
	Vec3 backBottomLeft		= minTilePosition + Vec3::UNIT_POSITIVE_Y;
	Vec3 backBottomRight	= backBottomLeft + Vec3::UNIT_POSITIVE_X;
	Vec3 backTopLeft		= backBottomLeft + Vec3::UNIT_POSITIVE_Z;
	Vec3 backTopRight		= backBottomLeft + Vec3( 1.f, 0.f, 1.f );

	Vec3 frontTangent	= ( frontBottomRight - frontBottomLeft ).GetNormalize();
	Vec3 frontBitangent = ( frontTopLeft - frontBottomLeft ).GetNormalize();
	Vec3 frontNormal	= CrossProduct3D( frontTangent, frontBitangent );

	MapRegion* tileRegionType	= m_tiles[tileIndex]->GetRegionType();
	MapMaterial* sideMaterial	= tileRegionType->GetSideMaterial();
	AABB2 sideUVBox				= sideMaterial->GetUVBox();
	Vec2 sideTopLeftUV			= Vec2( sideUVBox.mins.x, sideUVBox.maxes.y);
	Vec2 sideBottomRightUV		= Vec2( sideUVBox.maxes.x, sideUVBox.mins.y);

	Tile* northTile = GetTileInDirectionFromTileIndex( tileIndex, 0, 1 ); 
	Tile* eastTile = GetTileInDirectionFromTileIndex( tileIndex, 1, 0 );
	Tile* southTile = GetTileInDirectionFromTileIndex( tileIndex, 0, -1 );
	Tile* westTile = GetTileInDirectionFromTileIndex( tileIndex, -1, 0 );

	//Front - South Face
	if( !IsTileSolid( southTile ) )
	{
		m_mapVerts.push_back(	Vertex_PCUTBN( frontBottomLeft,		color,	frontTangent,	frontBitangent,		frontNormal,		sideUVBox.mins ) );
		m_mapVerts.push_back(	Vertex_PCUTBN( frontBottomRight,	color,	frontTangent,	frontBitangent,		frontNormal,		sideBottomRightUV ) );
		m_mapVerts.push_back(	Vertex_PCUTBN( frontTopRight,		color,	frontTangent,	frontBitangent,		frontNormal,		sideUVBox.maxes ) );
		
		m_mapVerts.push_back(	Vertex_PCUTBN( frontBottomLeft,		color,	frontTangent,	frontBitangent,		frontNormal,		sideUVBox.mins ) );
		m_mapVerts.push_back(	Vertex_PCUTBN( frontTopRight,		color,	frontTangent,	frontBitangent,		frontNormal,		sideUVBox.maxes ) );
		m_mapVerts.push_back(	Vertex_PCUTBN( frontTopLeft,		color,	frontTangent,	frontBitangent,		frontNormal,		sideTopLeftUV ) );
	}

	//Right - East Face
	if( !IsTileSolid( eastTile ) )
	{
		m_mapVerts.push_back(	Vertex_PCUTBN( frontBottomRight,	color,	-frontNormal,	frontBitangent,		frontTangent,		sideUVBox.mins ) );
		m_mapVerts.push_back(	Vertex_PCUTBN( backBottomRight,		color,	-frontNormal,	frontBitangent,		frontTangent,		sideBottomRightUV ) );
		m_mapVerts.push_back(	Vertex_PCUTBN( backTopRight,		color,	-frontNormal,	frontBitangent,		frontTangent,		sideUVBox.maxes ) );																											 		    

		m_mapVerts.push_back(	Vertex_PCUTBN( frontBottomRight,	color,	-frontNormal,	frontBitangent,		frontTangent,		sideUVBox.mins ) );
		m_mapVerts.push_back(	Vertex_PCUTBN( backTopRight,		color,	-frontNormal,	frontBitangent,		frontTangent,		sideUVBox.maxes ) );																											 		    
		m_mapVerts.push_back(	Vertex_PCUTBN( frontTopRight,		color,	-frontNormal,	frontBitangent,		frontTangent,		sideTopLeftUV ) );
	}

	//Back - North Face
	if( !IsTileSolid( northTile ) )
	{
		m_mapVerts.push_back(	Vertex_PCUTBN( backBottomRight,		color,	-frontTangent,	frontBitangent,		-frontNormal,		sideUVBox.mins ) );
		m_mapVerts.push_back(	Vertex_PCUTBN( backBottomLeft,		color,	-frontTangent,	frontBitangent,		-frontNormal,		sideBottomRightUV ) );
		m_mapVerts.push_back(	Vertex_PCUTBN( backTopLeft,			color,	-frontTangent,	frontBitangent,		-frontNormal,		sideUVBox.maxes ) );

		m_mapVerts.push_back(	Vertex_PCUTBN( backBottomRight,		color,	-frontTangent,	frontBitangent,		-frontNormal,		sideUVBox.mins ) );
		m_mapVerts.push_back(	Vertex_PCUTBN( backTopLeft,			color,	-frontTangent,	frontBitangent,		-frontNormal,		sideUVBox.maxes ) );																											 		      
		m_mapVerts.push_back(	Vertex_PCUTBN( backTopRight,		color,	-frontTangent,	frontBitangent,		-frontNormal,		sideTopLeftUV ) );
	}

	//Left - West Face
	if( !IsTileSolid( westTile ) )
	{
		m_mapVerts.push_back(	Vertex_PCUTBN( backBottomLeft,		color,	frontNormal,	frontBitangent,		-frontTangent,		sideUVBox.mins ) );
		m_mapVerts.push_back(	Vertex_PCUTBN( frontBottomLeft,		color,	frontNormal,	frontBitangent,		-frontTangent,		sideBottomRightUV ) );
		m_mapVerts.push_back(	Vertex_PCUTBN( frontTopLeft,		color,	frontNormal,	frontBitangent,		-frontTangent,		sideUVBox.maxes ) );

		m_mapVerts.push_back(	Vertex_PCUTBN( backBottomLeft,		color,	frontNormal,	frontBitangent,		-frontTangent,		sideUVBox.mins ) );
		m_mapVerts.push_back(	Vertex_PCUTBN( frontTopLeft,		color,	frontNormal,	frontBitangent,		-frontTangent,		sideUVBox.maxes ) );																									 		      
		m_mapVerts.push_back(	Vertex_PCUTBN( backTopLeft,			color,	frontNormal,	frontBitangent,		-frontTangent,		sideTopLeftUV ) );
	}
}


//---------------------------------------------------------------------------------------------------------
void TileMap::SpawnEntity( EntityType entityType )
{
	UNUSED( entityType );
}


//---------------------------------------------------------------------------------------------------------
void TileMap::CreateFromXML( XmlElement const& xmlElement )
{
	bool wasLegendRead		= false;
	bool wasMapRead			= false;
	bool wasEntitiesRead	= false;

	m_dimensions	= ParseXmlAttribute( xmlElement, "dimensions", m_dimensions );
	int version		= ParseXmlAttribute( xmlElement, "version", 0 );

	size_t mapSize = static_cast<size_t>( m_dimensions.x * m_dimensions.y );
	m_tiles.resize( mapSize );

	XmlElement const* nextChildElement = xmlElement.FirstChildElement();
	for( ;; )
	{
		if( nextChildElement == nullptr )
			break;

		std::string elementName = nextChildElement->Name();
		if( elementName == "Legend" )
		{
			CreateLegendFromXML( *nextChildElement );
		}
		else if( elementName == "MapRows" )
		{
			CreateTilesFromXML( *nextChildElement );
		}
		else if( elementName == "Entities" )
		{
			CreateEntitiesFromXML( *nextChildElement );
		}
		else
		{
			g_theConsole->ErrorString( "Invalid Element Name Found: %s", elementName.c_str() );
		}

		nextChildElement = nextChildElement->NextSiblingElement();
	}
}


//---------------------------------------------------------------------------------------------------------
bool TileMap::IsTileSolid( Tile* tile ) const
{
	if( tile == nullptr )
	{
		return true;
	}
	return tile->IsSolid();
}