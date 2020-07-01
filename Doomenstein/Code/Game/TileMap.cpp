#include "Game/TileMap.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapRegion.hpp"
#include "Game/MapMaterial.hpp"
#include "Game/Tile.hpp"
#include "Game/EntityDef.hpp"
#include "Game/Entity.hpp"
#include "Game/Game.hpp"
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
RaycastResult TileMap::Raycast( Vec3 const& startPosition, Vec3 const& fwdDir, float maxDistance )
{
	std::vector<RaycastResult> results;
	results.push_back( RaycastAgainstCeilingAndFloor( startPosition, fwdDir, maxDistance ) );
	results.push_back( RaycastAgainstWalls( startPosition, fwdDir, maxDistance ) );
	results.push_back( RaycastAgainstEntities( startPosition, fwdDir, maxDistance ) );
	return GetBestRaycast( results );
}


//---------------------------------------------------------------------------------------------------------
RaycastResult TileMap::RaycastAgainstCeilingAndFloor( Vec3 const& startPosition, Vec3 const& fwdDir, float maxDistance )
{
	const float floorHeight = 0.f;
	const float ceilingHeight = 1.f;

	Vec3 endPosition = startPosition + ( fwdDir * maxDistance );
	if( endPosition.z < ceilingHeight && endPosition.z > floorHeight )
		return RaycastResult( startPosition, fwdDir, maxDistance, endPosition, false, maxDistance );

	Vec3 hitNormal = Vec3::ZERO;
	float hitDistance = maxDistance;
	if( fwdDir.z > 0.f )
	{
		float fractionToCeiling = ( ceilingHeight - startPosition.z ) / ( endPosition.z - startPosition.z );
		hitDistance = maxDistance * fractionToCeiling;
		hitNormal = Vec3::UNIT_NEGATIVE_Z;
	}
	else if( fwdDir.z < 0.f )
	{
		float fractionToFloor = ( floorHeight - startPosition.z ) / ( endPosition.z - startPosition.z );
		hitDistance = maxDistance * fractionToFloor;
		hitNormal = Vec3::UNIT_POSITIVE_Z;
	}

	Vec3 hitPosition = startPosition + ( fwdDir * hitDistance );
	return RaycastResult( startPosition, fwdDir, maxDistance, hitPosition, true, hitDistance, hitNormal );
}


//---------------------------------------------------------------------------------------------------------
RaycastResult TileMap::RaycastAgainstWalls( Vec3 const& startPosition, Vec3 const& fwdDir, float maxDistance )
{
	IntVec2 currentTileCoords;
	currentTileCoords.x = RoundDownToInt( startPosition.x );
	currentTileCoords.y = RoundDownToInt( startPosition.y );

	Tile* tile = GetTileByCoords( currentTileCoords );
	if( IsTileSolid( tile ) )
	{
		return RaycastResult( startPosition, fwdDir, maxDistance, startPosition, true, 0.f, -fwdDir );
	}

	Vec3 rayDisplacement = fwdDir * maxDistance;
	int xStepSign = static_cast<int>( Signf( rayDisplacement.x ) );
	int yStepSign = static_cast<int>( Signf( rayDisplacement.y ) );

	float xStep = 1.f / abs( rayDisplacement.x );
	float yStep = 1.f / abs( rayDisplacement.y );
	
	int offsetToLeadingEdgeX = ( xStepSign + 1 ) / 2;
	int offsetToLeadingEdgeY = ( yStepSign + 1 ) / 2;
	
	float firstIntersectionX = static_cast<float>( currentTileCoords.x + offsetToLeadingEdgeX );
	float firstIntersectionY = static_cast<float>( currentTileCoords.y + offsetToLeadingEdgeY );
	
	float nextCrossingX = abs( firstIntersectionX - startPosition.x ) * xStep;
	float nextCrossingY = abs( firstIntersectionY - startPosition.y ) * yStep;

	for( ;; )
	{
		if( nextCrossingX <= nextCrossingY )
		{
			if( nextCrossingX > 1.f )
				return RaycastResult( startPosition, fwdDir, maxDistance, startPosition + rayDisplacement, false, maxDistance );

			currentTileCoords.x += xStepSign;
			tile = GetTileByCoords( currentTileCoords );
			if( IsTileSolid( tile ) )
			{
				Vec3 impactPosition = startPosition + ( fwdDir * ( maxDistance * nextCrossingX ) );
				return RaycastResult( startPosition, fwdDir, maxDistance, impactPosition, true, maxDistance * nextCrossingX, Vec3( (float)(-xStepSign), 0.f, 0.f ) );
			}
			nextCrossingX += xStep;
		}
		else if( nextCrossingY < nextCrossingX )
		{
			if( nextCrossingY > 1.f )
				return RaycastResult( startPosition, fwdDir, maxDistance, startPosition + rayDisplacement, false, maxDistance );

			currentTileCoords.y += yStepSign;
			tile = GetTileByCoords( currentTileCoords );
			if( IsTileSolid( tile ) )
			{
				Vec3 impactPosition = startPosition + ( fwdDir * ( maxDistance * nextCrossingY ) );
				return RaycastResult( startPosition, fwdDir, maxDistance, impactPosition, true, maxDistance * nextCrossingY, Vec3( 0.f, (float)(-yStepSign), 0.f ) );
			}
			nextCrossingY += yStep;
		}
	}
}

//---------------------------------------------------------------------------------------------------------
RaycastResult TileMap::RaycastAgainstEntities( Vec3 const& startPosition, Vec3 const& fwdDir, float maxDistance )
{
	Entity* impactedEntity = nullptr;
	Vec3 closestOverallImpact;
	float closestOverallDistance = 0.f;

	for( uint entityIndex = 0; entityIndex < m_entities.size(); ++entityIndex )
	{
		std::vector<Vec3> potentialHitPoints;
		//xy overlap check
		Entity* currentEntity = m_entities[ entityIndex ];
		if( currentEntity == nullptr || currentEntity->IsPossessed() )
			continue;

		Vec3 entityPosition = currentEntity->GetPosition();
		float entityRadius = currentEntity->GetPhysicsRadius();
		
		Vec2 startPositionXY = Vec2( startPosition.x, startPosition.y );
		Vec2 entityPositionXY = Vec2( entityPosition.x, entityPosition.y );
		if( !DoDiscsOverlap( startPositionXY, maxDistance, entityPositionXY, entityRadius ) )
			continue;

		Vec2 fwdDirIBasis = Vec2( fwdDir.x, fwdDir.y ).GetNormalized();
		Vec2 fwdDirJBasis = fwdDirIBasis.GetRotated90Degrees();

		Vec2 displacementToEntity = entityPositionXY - startPositionXY;

		float projectedIDistance = DotProduct2D( displacementToEntity, fwdDirIBasis );
		float distanceToRay = -DotProduct2D( displacementToEntity, fwdDirJBasis );

		float entityRadiusSquared = entityRadius * entityRadius;
		float distanceToRaySquared = distanceToRay * distanceToRay;
		float underRadicalValue = entityRadiusSquared - distanceToRaySquared;
		float plusMinusDistance = sqrtf( underRadicalValue );
		if( underRadicalValue > 0.f )
		{
			float distance1 = projectedIDistance - plusMinusDistance;
			float distance2 = projectedIDistance + plusMinusDistance;
			potentialHitPoints.push_back( startPosition + ( fwdDir * distance1 ) );
			potentialHitPoints.push_back( startPosition + ( fwdDir * distance2 ) );
		}
		else if( underRadicalValue == 0.f )
		{
			float distance = projectedIDistance;
			potentialHitPoints.push_back( startPosition + ( fwdDir * distance ) );
		}
		else
		{
			continue;
		}

		std::vector<Vec3> impactPoints = GetRayImpactPointsSideView( currentEntity, potentialHitPoints );
		if( impactPoints.size() > 0 )
		{
			Vec3 closestImpact = GetClosestPointFromList( startPosition, impactPoints );
			float distanceToImpact = GetDistance3D( startPosition, closestImpact );
			if( impactedEntity == nullptr || distanceToImpact < closestOverallDistance )
			{
				impactedEntity = currentEntity;
				closestOverallImpact = closestImpact;
				closestOverallDistance = distanceToImpact;
			}
		}
	}

	if( impactedEntity != nullptr )
	{
		Vec2 fwdDirXY = Vec2( fwdDir.x, fwdDir.y );
		Vec2 impactNormalXY = fwdDirXY.GetRotatedMinus90Degrees();
		Vec3 impactNormal = Vec3( impactNormalXY, 0.f );
		return RaycastResult( startPosition, fwdDir, maxDistance, closestOverallImpact, true, closestOverallDistance, impactNormal, impactedEntity );
	}
	return RaycastResult( startPosition, fwdDir, maxDistance, startPosition + ( fwdDir * maxDistance ), false, maxDistance );
}


//---------------------------------------------------------------------------------------------------------
RaycastResult TileMap::GetBestRaycast( std::vector<RaycastResult> const& results )
{
	RaycastResult bestRaycast;
	float bestRaycastResultImpactDistance = (float)0xffffffff;
	for( uint resultsIndex = 0; resultsIndex < results.size(); ++resultsIndex )
	{
		RaycastResult result = results[ resultsIndex ];
		if( result.impactDistance < bestRaycastResultImpactDistance )
		{
			bestRaycastResultImpactDistance = result.impactDistance;
			bestRaycast = result;
		}
	}
	return bestRaycast;
}


//---------------------------------------------------------------------------------------------------------
std::vector<Vec3> TileMap::GetRayImpactPointsSideView( Entity* entity, std::vector<Vec3> const& potentialHits )
{
	std::vector<Vec3> impactPoints;

	float entityRadius = entity->GetPhysicsRadius();
	float entityHeight = entity->GetHeight();
	Vec3 entityPosition = entity->GetPosition();

	Vec2 XZBoundsMins = Vec2( entityPosition.x - entityRadius, entityPosition.z );
	Vec2 XZBoundsMaxes = Vec2( entityPosition.x + entityRadius, entityPosition.z + entityHeight );
	AABB2 entityXZBounds = AABB2( XZBoundsMins, XZBoundsMaxes );
	
	for( uint potentialHitIndex = 0; potentialHitIndex < potentialHits.size(); ++potentialHitIndex )
	{
		Vec3 hitPoint = potentialHits[ potentialHitIndex ];
		Vec2 hitPointXZ = Vec2( hitPoint.x, hitPoint.z );

		if( IsPointInsideAABB2D( hitPointXZ, entityXZBounds ) )
		{
			impactPoints.push_back( hitPoint );
		}
	}
	return impactPoints;
}


//---------------------------------------------------------------------------------------------------------
Vec3 TileMap::GetClosestPointFromList( Vec3 const& point, std::vector<Vec3> const& pointsToCheck )
{
	Vec3 closestPoint;
	//int 
	float closestDistanceSquaredToPoint = (float)0xffffffff;
	for( uint pointIndex = 0; pointIndex < pointsToCheck.size(); ++pointIndex )
	{
		Vec3 currentPoint = pointsToCheck[ pointIndex ];
		float distanceSquared = GetDistanceSquared3D( point, currentPoint );
		if( distanceSquared < closestDistanceSquaredToPoint )
		{
			closestDistanceSquaredToPoint = distanceSquared;
			closestPoint = currentPoint;
		}
	}
	return closestPoint;
}


//---------------------------------------------------------------------------------------------------------
void TileMap::Update()
{
	CreateMapVerts();
	UpdateEntities();
	HandleEntityVEntityCollisions();
	HandleEntitiesVWallCollisions();
}


//---------------------------------------------------------------------------------------------------------
void TileMap::UpdateEntities()
{
	for( uint entityIndex = 0; entityIndex < m_entities.size(); ++entityIndex )
	{
		Entity* currentEntity = m_entities[ entityIndex ];
		if( currentEntity != nullptr )
		{
			currentEntity->Update();
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void TileMap::Render() const
{
	RenderMap();
	RenderEntities();
}


//---------------------------------------------------------------------------------------------------------
void TileMap::RenderMap() const
{
	g_theRenderer->BindTextureByPath( "Data/Images/Terrain_8x8.png" );
	g_theRenderer->BindShader( (Shader*)nullptr );

	g_theRenderer->DrawMesh( m_mapMesh );
}


//---------------------------------------------------------------------------------------------------------
void TileMap::RenderEntities() const
{
	for( uint entityIndex = 0; entityIndex < m_entities.size(); ++entityIndex )
	{
		Entity* currentEntity = m_entities[ entityIndex ];
		if( currentEntity != nullptr )
		{
			currentEntity->Render();
		}
	}
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
		else
		{
			g_theConsole->ErrorString( "Unsupported element type %s in Legend", elementName.c_str() );
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

		if( yPos < 0 )
		{
			g_theConsole->ErrorString( "The rows in 'MapRows' are greater than the specified dimension of %i", m_dimensions.y );
			return;
		}

		if( !IsStringEqual( nextChildElement->Name(), "MapRow" ) )
		{
			g_theConsole->ErrorString( "Unsupported 'MapRows' Element: %s",  nextChildElement->Name() );
			return;
		}

		std::string mapRow = ParseXmlAttribute( *nextChildElement, "tiles", "MISSING" );
		if( mapRow == "MISSING" )
		{
			g_theConsole->ErrorString( "Failed to parse 'MapRow' attribute: 'tiles' at line #%i", nextChildElement->GetLineNum() );
			return;
		}
		else if( mapRow.length() != m_dimensions.x )
		{
			g_theConsole->ErrorString( "'MapRow' at line %i of length %i does not match specified dimension of %i", nextChildElement->GetLineNum(), mapRow.length(), m_dimensions.x );
			return;
		}

		for( int glyphIndex = 0; glyphIndex < mapRow.length(); ++glyphIndex )
		{
			char glyph = mapRow[ glyphIndex ];
			AddTileByLegendGlyph( glyph, glyphIndex, yPos );
		}
		nextChildElement = nextChildElement->NextSiblingElement();
		--yPos;
	}

	if( yPos > 0 )
	{
		g_theConsole->ErrorString( "The rows in 'MapRows' are less than the specified dimension of %i", m_dimensions.y );
		return;
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
		EntityType entityTypeToSpawn = EntityDef::GetEntityTypeFromString( entityTypeAsString );
		
		switch( entityTypeToSpawn )
		{
		case ENTITY_TYPE_ENTITY:
		case ENTITY_TYPE_ACTOR:
		case ENTITY_TYPE_PORTAL:
		case ENTITY_TYPE_PROJECTILE:
			SpawnNewEntityOnMap( *nextChildElement );
			break;
		default:
			if( entityTypeAsString == "PlayerStart" )
			{
				CreatePlayerStart( *nextChildElement );
			}
			else
			{
				g_theConsole->ErrorString( "%s is an unsupported entity type", entityTypeAsString.c_str() );
			}
			break;
		}

		nextChildElement = nextChildElement->NextSiblingElement();
	}
}


//---------------------------------------------------------------------------------------------------------
void TileMap::HandleEntitiesVWallCollisions()
{
	for( uint entityIndex = 0; entityIndex < m_entities.size(); ++entityIndex )
	{
		Entity* currentEntity = m_entities[ entityIndex ];
		HandleEntityVWallCollisions( currentEntity );
	}
}


//---------------------------------------------------------------------------------------------------------
void TileMap::HandleEntityVWallCollisions( Entity* entity )
{
	PushEntityOutOfWall( entity,  0,  0 ); // Current
	PushEntityOutOfWall( entity,  1,  0 ); // East
	PushEntityOutOfWall( entity,  1,  1 ); // North-East
	PushEntityOutOfWall( entity,  0,  1 ); // North
	PushEntityOutOfWall( entity, -1,  1 ); // North-West
	PushEntityOutOfWall( entity, -1,  0 ); // West
	PushEntityOutOfWall( entity, -1, -1 ); // South-West
	PushEntityOutOfWall( entity,  0, -1 ); // South
	PushEntityOutOfWall( entity,  1, -1 ); // South-East
}


//---------------------------------------------------------------------------------------------------------
void TileMap::PushEntityOutOfWall( Entity* entity, int xDir, int yDir )
{
	float entityRadius = entity->GetPhysicsRadius();
	Vec3 entityPosition = entity->GetPosition();
	Vec2 entityPositionXY = Vec2( entityPosition.x, entityPosition.y );

	IntVec2 currentTileCoords;
	currentTileCoords.x = RoundDownToInt( entityPositionXY.x );
	currentTileCoords.y = RoundDownToInt( entityPositionXY.y );

	IntVec2 tileToCheckCoords = currentTileCoords + IntVec2( xDir, yDir );
	Tile* tileToCheck = GetTileByCoords( tileToCheckCoords );
	
	AABB2 tileXYBounds = GetTileXYBounds( tileToCheck );

	if( IsTileSolid( tileToCheck ) && DoDiscAndAABB2Overlap( tileXYBounds, entityPositionXY, entityRadius ) )
	{
		PushDiscOutOfAABB2( entityPositionXY, entityRadius, tileXYBounds );
		entity->SetPosition( Vec3( entityPositionXY, entityPosition.z ) );
	}
}

//---------------------------------------------------------------------------------------------------------
void TileMap::AddGlyphToLegend( XmlElement const& xmlElement )
{
	char tileGlyph = ParseXmlAttribute( xmlElement, "glyph", '`' );
	if( tileGlyph == '`' )
	{
		g_theConsole->ErrorString( "Failed to parse Legend element attribute: 'glyph' at line #%i", xmlElement.GetLineNum() );
		return;
	}
	
	std::string regionTypeName = ParseXmlAttribute( xmlElement, "regionType", "MISSING" );
	if( regionTypeName == "MISSING" )
	{
		g_theConsole->ErrorString( "Failed to parse Legend element attribute: 'regionType' at line #%i", xmlElement.GetLineNum() );
		return;
	}

	if( m_legend[ tileGlyph ] != "" )
	{
		g_theConsole->ErrorString( "The glyph '%c' cannot be used twice - will use %s", tileGlyph, m_legend[ tileGlyph ].c_str() );
		return;
	}
	
	m_legend[ tileGlyph ] = regionTypeName;
}


//---------------------------------------------------------------------------------------------------------
void TileMap::AddTileByLegendGlyph( char glyph, int xPosition, int yPosition )
{
	std::string regionTypeName = m_legend[ glyph ];
	if( regionTypeName == "" )
	{
		g_theConsole->ErrorString( "Did not find specified Map Region Type for '%c'", glyph );
	}

	MapRegion* regionType = MapRegion::GetRegionByName( regionTypeName );
	if( regionType == nullptr && regionTypeName != "" )
	{
		g_theConsole->ErrorString( "Did not find Map Region '%s' using glyph '%c'", regionTypeName.c_str(), glyph );
	}

	if( regionType == nullptr || regionTypeName == "" )
	{
		g_theConsole->ErrorString( "Using default material at (%i, %i)", xPosition, yPosition );
		regionType = MapRegion::GetDefaultRegion();
	}

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
AABB2 TileMap::GetTileXYBounds( Tile* tile ) const
{
	IntVec2 tileCoords = tile->GetTileCoords();
	
	Vec2 tilePosition;
	tilePosition.x = static_cast<float>( tileCoords.x );
	tilePosition.y = static_cast<float>( tileCoords.y );

	Vec2 tileMaxesXY = tilePosition + Vec2( 1.f, 1.f );
	
	return AABB2( tilePosition, tileMaxesXY );
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
	m_playerStartPositionXY = playerStartPositionXY;
	m_playerStartYaw = playerStartYawDegrees;
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
		if( m_tiles[ tileIndex ] != nullptr )
			AppendVertsForTile( tileIndex );
 	}

	if( m_mapVerts.size() > 0 )
	{
		m_mapMesh->UpdateVerticies( static_cast<uint>( m_mapVerts.size() ), &m_mapVerts[0] );
	}
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

	AABB2		ceilingUVBox	= AABB2( 0.f, 0.f, 1.f, 1.f );
	AABB2		floorUVBox		= AABB2( 0.f, 0.f, 1.f, 1.f );
	MapRegion*	tileRegionType	= m_tiles[ tileIndex ]->GetRegionType();

	if( tileRegionType != nullptr )
	{
		MapMaterial* ceilingMaterial	= tileRegionType->GetCeilingMaterial();
		MapMaterial* floorMaterial		= tileRegionType->GetFloorMaterial();
		
		if( ceilingMaterial )
			ceilingUVBox = ceilingMaterial->GetUVBox();
		if( floorMaterial )
			floorUVBox = floorMaterial->GetUVBox();
	}
	Vec2 ceilingTopLeftUV			= Vec2( ceilingUVBox.mins.x, ceilingUVBox.maxes.y );
	Vec2 ceilingBottomRightUV		= Vec2( ceilingUVBox.maxes.x, ceilingUVBox.mins.y );
	Vec2 floorTopLeftUV				= Vec2( floorUVBox.mins.x, floorUVBox.maxes.y );
	Vec2 floorBottomRightUV			= Vec2( floorUVBox.maxes.x, floorUVBox.mins.y );

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

	AABB2		sideUVBox		= AABB2( 0.f, 0.f, 1.f, 1.f );
	MapRegion*	tileRegionType	= m_tiles[tileIndex]->GetRegionType();
	if( tileRegionType != nullptr )
	{
		MapMaterial* sideMaterial = tileRegionType->GetSideMaterial();
		sideUVBox = sideMaterial->GetUVBox();
	}
	Vec2 sideTopLeftUV		= Vec2( sideUVBox.mins.x, sideUVBox.maxes.y);
	Vec2 sideBottomRightUV	= Vec2( sideUVBox.maxes.x, sideUVBox.mins.y);

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
void TileMap::CreateFromXML( XmlElement const& xmlElement )
{
	bool	wasLegendRead		= false;
	bool	wasMapRead			= false;
	bool	wasEntitiesRead		= false;
	
	m_dimensions = ParseXmlAttribute( xmlElement, "dimensions", m_dimensions );
	if( m_dimensions == IntVec2( -1, -1 ) )
	{
		g_theConsole->ErrorString( "Could not parse attribute: 'dimensions' at line #%i", xmlElement.GetLineNum() );
		return;
	}

	int version	= ParseXmlAttribute( xmlElement, "version", 0 );
	if( version == 0 )
	{
		g_theConsole->ErrorString( "Could not parse attribute: 'version' at line #%i", xmlElement.GetLineNum() );
		return;
	}

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
			if( !wasLegendRead )
			{
				CreateLegendFromXML( *nextChildElement );
				wasLegendRead = true;
			}
			else
			{
				g_theConsole->ErrorString( "Only one 'Legend' is allowed per TileMap" );
			}
		}
		else if( elementName == "MapRows" )
		{
			if( !wasMapRead )
			{
				CreateTilesFromXML( *nextChildElement );
				wasMapRead = true;
			}
			else
			{
				g_theConsole->ErrorString( "Only one 'MapRows' element is allowed per TileMap" );
			}
		}
		else if( elementName == "Entities" )
		{
			if (!wasEntitiesRead)
			{
				CreateEntitiesFromXML( *nextChildElement );
				wasEntitiesRead = true;
			}
			else
			{
				g_theConsole->ErrorString( "Only one 'Entities' element is allowed per TileMap" );
			}
		}
		else
		{
			g_theConsole->ErrorString(							"Unsupported Element Name Found: %s at line #%i", elementName.c_str(), nextChildElement->GetLineNum() );
			g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR,	"Supported Element Types:" );
			g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR,	"  Legend" );
			g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR,	"  MapRows" );
			g_theConsole->PrintString( DEV_CONSOLE_HELP_COLOR,	"  Entities" );
		}

		nextChildElement = nextChildElement->NextSiblingElement();
	}

	if( !wasLegendRead )
	{
		g_theConsole->ErrorString( "A TileMap must have an 'Legend' element" );
	}
	if( !wasMapRead )
	{
		g_theConsole->ErrorString( "A TileMap must have an 'MapRows' element" );
	}
	if( !wasEntitiesRead )
	{
		g_theConsole->ErrorString( "A TileMap must have an 'Entities' element" );
	}
}


//---------------------------------------------------------------------------------------------------------
bool TileMap::IsTileSolid( Tile* tile ) const
{
	if( tile == nullptr || tile->GetRegionType() == nullptr )
	{
		return true;
	}
	return tile->IsSolid();
}