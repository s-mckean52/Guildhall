#include "Game/Entity.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/Mat44.hpp"


//---------------------------------------------------------------------------------------------------------
Entity::Entity( Game* theGame, World* theWorld, Map* theMap, EntityDef const& entityDef )
	: m_entityDef( entityDef )
{
	m_theGame = theGame;
	m_theWorld = theWorld;
	m_theMap = theMap;

	Vec2 spriteDimensions = m_entityDef.GetSize();
	m_bottomLeft	= Vec3( 0.f, -spriteDimensions.x * 0.5f, 0.f );
	m_bottomRight	= Vec3( 0.f, spriteDimensions.x * 0.5f, 0.f );
	m_topRight		= Vec3( 0.f, spriteDimensions.x * 0.5f, spriteDimensions.y );
	m_topLeft		= Vec3( 0.f, -spriteDimensions.x * 0.5f, spriteDimensions.y );
}


//---------------------------------------------------------------------------------------------------------
Entity::~Entity()
{
}


//---------------------------------------------------------------------------------------------------------
void Entity::Update()
{
	if( m_isPossessed )
		return;

	m_forwardDirection = Vec2::MakeFromPolarDegrees( m_yaw ); 
	UpdateAnimDirection(); //Should happen last
}


//---------------------------------------------------------------------------------------------------------
void Entity::Render() const
{
	if( m_isPossessed )
		return;

	SpriteSheet* spriteSheet = m_entityDef.GetSpriteSheet();

	if( spriteSheet != nullptr )
	{
		SpriteDefinition const* currentSpriteDef = m_entityDef.GetSpriteDefinitionForAnimSetAtTime( m_actionState, m_currentSpriteDirection, 0.f );
		BillboardType billboardType = m_entityDef.GetBillBoardType();
		Mat44 billboardTransform = GetBillboardTransformMatrix( *m_theGame->GetPlayerCamera(), m_position, billboardType );
		
		Vec3 bottomLeft		= billboardTransform.TransformPosition3D( m_bottomLeft );
		Vec3 bottomRight	= billboardTransform.TransformPosition3D( m_bottomRight );
		Vec3 topRight		= billboardTransform.TransformPosition3D( m_topRight );
		Vec3 topLeft		= billboardTransform.TransformPosition3D( m_topLeft );

		Vec2 bottomLeftUV;
		Vec2 topRightUV;
		currentSpriteDef->GetUVs( bottomLeftUV, topRightUV );
		Vec2 bottomRightUV	= Vec2( topRightUV.x, bottomLeftUV.y );
		Vec2 topLeftUV		= Vec2( bottomLeftUV.x, topRightUV.y );

		std::vector<Vertex_PCU> vertexArray;

		vertexArray.push_back( Vertex_PCU( bottomLeft,	Rgba8::WHITE,	bottomLeftUV	) );
		vertexArray.push_back( Vertex_PCU( bottomRight, Rgba8::WHITE,	bottomRightUV	) );
		vertexArray.push_back( Vertex_PCU( topRight,	Rgba8::WHITE,	topRightUV		) );

		vertexArray.push_back( Vertex_PCU( bottomLeft,	Rgba8::WHITE,	bottomLeftUV	) );
		vertexArray.push_back( Vertex_PCU( topRight,	Rgba8::WHITE,	topRightUV		) );
		vertexArray.push_back( Vertex_PCU( topLeft,		Rgba8::WHITE,	topLeftUV		) );

		g_theRenderer->BindTexture( &spriteSheet->GetTexture() );
		g_theRenderer->BindShaderByPath( "Data/Shaders/WorldOpaque.hlsl" );

		//TranslateVertexArray( vertexArray, m_position );
		g_theRenderer->DrawVertexArray( vertexArray );
	}

	if( g_isDebugDraw )
	{
		DebugRender();
	}
}


//---------------------------------------------------------------------------------------------------------
void Entity::DebugRender() const
{
	float radius = m_entityDef.GetRadius();

	Vec3 cylinderTopPosition = m_position;
	cylinderTopPosition.z += m_entityDef.GetHeight();

	Vec3 eyeLineStartPos = m_position + Vec3( 0.f, 0.f, m_entityDef.GetEyeHeight() );
	Vec3 eyeLineDirection = Vec3( m_forwardDirection, 0.f );

	BillboardType billboardType = m_entityDef.GetBillBoardType();
	Mat44 billboardTransform = GetBillboardTransformMatrix( *m_theGame->GetPlayerCamera(), m_position, billboardType );
	Vec3 bottomLeft		= billboardTransform.TransformPosition3D( m_bottomLeft );
	Vec3 bottomRight	= billboardTransform.TransformPosition3D( m_bottomRight );
	Vec3 topRight		= billboardTransform.TransformPosition3D( m_topRight );
	Vec3 topLeft		= billboardTransform.TransformPosition3D( m_topLeft );

	//DebugAddWorldBillboardTextf( m_position + Vec3( 0.f, 0.f, m_entityDef.GetHeight() ), ALIGN_BOTTOM_CENTERED, Rgba8::WHITE, "%s", m_currentSpriteDirection.c_str() );

	DebugAddWorldLine( bottomLeft, bottomRight, Rgba8::RED, 0.f, DEBUG_RENDER_ALWAYS );
	DebugAddWorldLine( bottomRight, topRight, Rgba8::YELLOW, 0.f, DEBUG_RENDER_ALWAYS );
	DebugAddWorldLine( topRight, topLeft, Rgba8::GREEN, 0.f, DEBUG_RENDER_ALWAYS );
	DebugAddWorldLine( topLeft, bottomLeft, Rgba8::ORANGE, 0.f, DEBUG_RENDER_ALWAYS );
	DebugAddWorldCone( m_position, radius, cylinderTopPosition, radius, Rgba8::CYAN, Rgba8::CYAN, 0.f, DEBUG_RENDER_ALWAYS );
	if( !m_isPossessed )
	{
		DebugAddWorldArrow( eyeLineStartPos, eyeLineStartPos + eyeLineDirection, Rgba8::CYAN, 0.f, DEBUG_RENDER_ALWAYS );
	}
}


//---------------------------------------------------------------------------------------------------------
EntityType Entity::GetEntityType() const
{
	return m_entityDef.GetEntityType();
}


//---------------------------------------------------------------------------------------------------------
float Entity::GetHeight() const
{
	return m_entityDef.GetHeight();
}


//---------------------------------------------------------------------------------------------------------
float Entity::GetEyeHeight() const
{
	return m_entityDef.GetEyeHeight();
}


//---------------------------------------------------------------------------------------------------------
float Entity::GetSpeed() const
{
	return m_entityDef.GetSpeed();
}


//---------------------------------------------------------------------------------------------------------
float Entity::GetPhysicsRadius() const
{
	return m_entityDef.GetRadius();
}


//---------------------------------------------------------------------------------------------------------
void Entity::UpdateAnimDirection()
{
	Camera* playerCamera = m_theGame->GetPlayerCamera();
	Vec3 cameraPosition = playerCamera->GetPosition();
	Vec2 cameraPositionXY = Vec2( cameraPosition.x, cameraPosition.y );
	Vec2 directionToCameraXY = cameraPositionXY - Vec2( m_position.x, m_position.y );

	Vec2 leftDirection = m_forwardDirection.GetRotated90Degrees();

	Vec2 localDirToCamera;
	localDirToCamera.x = DotProduct2D( directionToCameraXY, m_forwardDirection );
	localDirToCamera.y = DotProduct2D( directionToCameraXY, leftDirection );

	CheckAndUpdateSpriteDirection( localDirToCamera, "front",		Vec2(  1.f,				 0.f ) );
	CheckAndUpdateSpriteDirection( localDirToCamera, "back",		Vec2( -1.f,				 0.f ) );
	CheckAndUpdateSpriteDirection( localDirToCamera, "left",		Vec2(  0.f,				 1.f ) );
	CheckAndUpdateSpriteDirection( localDirToCamera, "right",		Vec2(  0.f,				-1.f ) );
	CheckAndUpdateSpriteDirection( localDirToCamera, "frontRight",	Vec2(  SQRT_2_OVER_2,	-SQRT_2_OVER_2 ) );
	CheckAndUpdateSpriteDirection( localDirToCamera, "frontLeft",	Vec2(  SQRT_2_OVER_2,	 SQRT_2_OVER_2 ) );
	CheckAndUpdateSpriteDirection( localDirToCamera, "backLeft",	Vec2( -SQRT_2_OVER_2,	 SQRT_2_OVER_2 ) );
	CheckAndUpdateSpriteDirection( localDirToCamera, "backRight",	Vec2( -SQRT_2_OVER_2,	-SQRT_2_OVER_2 ) );
}


//---------------------------------------------------------------------------------------------------------
void Entity::CheckAndUpdateSpriteDirection( Vec2 const& directionToCompare, std::string const& directionName, Vec2 const& direction )
{
	if( !m_entityDef.HasAnimationDirectionInSet( m_actionState, directionName ) )
		return;

	if( m_currentSpriteDirection == directionName )
		return;

	float currentDirectionValue = 0.f;
	if( m_currentSpriteDirection == "front" )			{ currentDirectionValue = DotProduct2D( directionToCompare, Vec2(  1.f,				 0.f ) ); }							  
	else if( m_currentSpriteDirection == "back"	)		{ currentDirectionValue = DotProduct2D( directionToCompare, Vec2( -1.f,				 0.f ) ); }
	else if( m_currentSpriteDirection == "left" )		{ currentDirectionValue = DotProduct2D( directionToCompare, Vec2(  0.f,				 1.f ) ); }
	else if( m_currentSpriteDirection == "right" )		{ currentDirectionValue = DotProduct2D( directionToCompare, Vec2(  0.f,				-1.f ) ); }
	else if( m_currentSpriteDirection == "frontRight" )	{ currentDirectionValue = DotProduct2D( directionToCompare, Vec2(  SQRT_2_OVER_2,	-SQRT_2_OVER_2 ) );	}
	else if( m_currentSpriteDirection == "frontLeft" )	{ currentDirectionValue = DotProduct2D( directionToCompare, Vec2(  SQRT_2_OVER_2,	 SQRT_2_OVER_2 ) );	}
	else if( m_currentSpriteDirection == "backLeft" )	{ currentDirectionValue = DotProduct2D( directionToCompare, Vec2( -SQRT_2_OVER_2,	 SQRT_2_OVER_2 ) );	}
	else if( m_currentSpriteDirection == "backRight" )	{ currentDirectionValue = DotProduct2D( directionToCompare, Vec2( -SQRT_2_OVER_2,	-SQRT_2_OVER_2 ) ); }
	else
		ERROR_AND_DIE( Stringf( "Bad direction name \"%s\" in updating direction", m_currentSpriteDirection.c_str() ) );

	float newDirectionValue = DotProduct2D( directionToCompare, direction );
	if( currentDirectionValue < newDirectionValue )
	{
		m_currentSpriteDirection = directionName;
	}
}

//---------------------------------------------------------------------------------------------------------
void Entity::SetPosition( Vec3 const& position )
{
	m_position = position;
}


//---------------------------------------------------------------------------------------------------------
void Entity::SetYaw( float yaw )
{
	m_yaw = yaw;
}


//---------------------------------------------------------------------------------------------------------
void Entity::Translate( Vec3 const& translation )
{
	m_position += translation;
}


//---------------------------------------------------------------------------------------------------------
void Entity::SetIsPossessed( bool isPossesed )
{
	m_isPossessed = isPossesed;
}
