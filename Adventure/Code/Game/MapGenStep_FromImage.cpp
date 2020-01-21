#include "Game/MapGenStep_FromImage.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/MathUtils.hpp"


//---------------------------------------------------------------------------------------------------------
MapGenStep_FromImage::MapGenStep_FromImage( const XmlElement& element )
	:MapGenStep( element )
{
	m_imageFilePath = ParseXmlAttribute( element, "ImageFilePath", m_imageFilePath );
	if( m_imageFilePath == "" )
	{
		ERROR_AND_DIE( "No File Path Specified for ImageFrom" );
	}

	m_alignmentX = ParseXmlAttribute( element, "AlignmentX", m_alignmentX );
	m_alignmentY = ParseXmlAttribute( element, "AlignmentY", m_alignmentY );
}


//---------------------------------------------------------------------------------------------------------
void MapGenStep_FromImage::RunStepOnce( Map& map )
{
	Image imageToUse = Image( m_imageFilePath.c_str() );
	IntVec2 imageDimensions = imageToUse.GetDimensions();

	Vec2 rolledAlignment;
	rolledAlignment.x = m_alignmentX.GetRandomInRange( *g_RNG );
	rolledAlignment.y = m_alignmentY.GetRandomInRange( *g_RNG );

	IntVec2 mapDimensions = map.GetMapDefinition()->GetDimensions();
	IntVec2 imageMaxMin = mapDimensions - imageDimensions;
	IntVec2 alignedStartTileCoord; //TODO: set based on read in alignment
	alignedStartTileCoord.x = static_cast<int>( static_cast<float>( imageMaxMin.x ) * rolledAlignment.x );
	alignedStartTileCoord.y = static_cast<int>( static_cast<float>( imageMaxMin.y ) * rolledAlignment.y );

	for( int texelPositionX = 0; texelPositionX < imageDimensions.x; ++texelPositionX )
	{
		for( int texelPositionY = 0; texelPositionY < imageDimensions.y; ++texelPositionY )
		{
			Rgba8 texelColor = imageToUse.GetTexelColor( IntVec2( texelPositionX, texelPositionY ) );

			int tileIndex = map.GetTileIndexForTileCoords( IntVec2( alignedStartTileCoord.x + texelPositionX, alignedStartTileCoord.y + texelPositionY ) );
			Tile& currentTile = map.GetTileAtIndex( tileIndex );

			currentTile.SetTileDefinitionWithTexel( texelColor );
		}
	}
}
