#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DebugRender.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Game/Entity.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/Projectile.hpp"


STATIC int Entity::s_entityID = 0;

//---------------------------------------------------------------------------------------------------------
Entity::Entity( Game* theGame, World* theWorld, Map* theMap, EntityDef const& entityDef, XmlElement const& element )
	: m_entityDef( entityDef )
{
	m_theGame = theGame;
	m_theWorld = theWorld;
	m_theMap = theMap;

	m_id = s_entityID++;

	Vec2 spriteDimensions = m_entityDef.GetSize();
	m_bottomLeft	= Vec3( 0.f, -spriteDimensions.x * 0.5f, 0.f );
	m_bottomRight	= Vec3( 0.f, spriteDimensions.x * 0.5f, 0.f );
	m_topRight		= Vec3( 0.f, spriteDimensions.x * 0.5f, spriteDimensions.y );
	m_topLeft		= Vec3( 0.f, -spriteDimensions.x * 0.5f, spriteDimensions.y );

	SetValuesFromXML( element );
}


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
// 	if( m_isPossessed )
// 		return;

	m_forwardDirection = Vec2::MakeFromPolarDegrees( m_yaw ); 
	UpdateAnimDirection(); //Should happen last
}


//---------------------------------------------------------------------------------------------------------
void Entity::Render() const
{
// 	if( m_isPossessed )
// 		return;

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

	RenderHealthBar();

	if( g_isDebugDraw )
	{
		DebugRender();
	}
}


//---------------------------------------------------------------------------------------------------------
void Entity::RenderHealthBar() const
{
	float healthHeight = 0.2f;
	float padding = 0.1f;
	float height = GetHeight();
	float radius = GetPhysicsRadius();
	float width = radius * 2.f;

	float healthSectionWidth = width / 10.f;

	Vec3 backgroundBottomLeft = Vec3( 0.f, -radius, height + padding );
	Vec3 backgroundBottomRight = Vec3( 0.f, radius, height + padding );
	Vec3 backgroundTopLeft = backgroundBottomLeft + Vec3( 0.f, 0.f, healthHeight );
	Vec3 backgroundTopRight = backgroundBottomRight + Vec3( 0.f, 0.f, healthHeight );

	Mat44 billboardTransform = GetBillboardTransformMatrix( *m_theGame->GetPlayerCamera(), m_position, BillboardType::CAMERA_FACING_XY );
	Vec3 t_backgroundBottomLeft		= billboardTransform.TransformPosition3D( backgroundBottomLeft );
	Vec3 t_backgroundBottomRight	= billboardTransform.TransformPosition3D( backgroundBottomRight );
	Vec3 t_backgroundTopRight		= billboardTransform.TransformPosition3D( backgroundTopRight );
	Vec3 t_backgroundTopLeft		= billboardTransform.TransformPosition3D( backgroundTopLeft );

	std::vector<Vertex_PCU> vertexArray;
	vertexArray.push_back( Vertex_PCU( t_backgroundBottomLeft,	Rgba8::GRAY ) );
	vertexArray.push_back( Vertex_PCU( t_backgroundBottomRight, Rgba8::GRAY ) );
	vertexArray.push_back( Vertex_PCU( t_backgroundTopRight,	Rgba8::GRAY ) );
	vertexArray.push_back( Vertex_PCU( t_backgroundBottomLeft,	Rgba8::GRAY ) );
	vertexArray.push_back( Vertex_PCU( t_backgroundTopRight,	Rgba8::GRAY ) );
	vertexArray.push_back( Vertex_PCU( t_backgroundTopLeft,		Rgba8::GRAY ) );
	
	g_theRenderer->BindTexture( nullptr );
	g_theRenderer->BindShaderByPath( "Data/Shaders/WorldOpaque.hlsl" );

	g_theRenderer->DrawVertexArray( vertexArray );

	std::vector<Vertex_PCU> healthSegments;
	for( int healthBarIndex = 0; healthBarIndex < m_currentHealth; ++healthBarIndex )
	{
		float segmentPadding = 0.01f;
		Vec3 segmentBottomLeft = backgroundBottomLeft + Vec3( 0.01f, ( healthSectionWidth * healthBarIndex ) + segmentPadding, segmentPadding );
		Vec3 segmentBottomRight = segmentBottomLeft + Vec3( 0.f, healthSectionWidth - ( segmentPadding * 2.f ), 0.f );
		Vec3 segmentTopLeft = segmentBottomLeft + Vec3( 0.f, 0.f, healthHeight - ( segmentPadding * 2.f ) );
		Vec3 segmentTopRight = segmentBottomRight + Vec3( 0.f, 0.f, healthHeight - ( segmentPadding * 2.f ) );

		Vec3 t_segmentBottomLeft	= billboardTransform.TransformPosition3D( segmentBottomLeft );
		Vec3 t_segmentBottomRight	= billboardTransform.TransformPosition3D( segmentBottomRight );
		Vec3 t_segmentTopRight		= billboardTransform.TransformPosition3D( segmentTopRight );
		Vec3 t_segmentTopLeft		= billboardTransform.TransformPosition3D( segmentTopLeft );

		healthSegments.push_back( Vertex_PCU( t_segmentBottomLeft,	Rgba8::GREEN ) );
		healthSegments.push_back( Vertex_PCU( t_segmentBottomRight,	Rgba8::GREEN ) );
		healthSegments.push_back( Vertex_PCU( t_segmentTopRight,	Rgba8::GREEN ) );
		healthSegments.push_back( Vertex_PCU( t_segmentBottomLeft,	Rgba8::GREEN ) );
		healthSegments.push_back( Vertex_PCU( t_segmentTopRight,	Rgba8::GREEN ) );
		healthSegments.push_back( Vertex_PCU( t_segmentTopLeft,		Rgba8::GREEN ) );
	}

	if( healthSegments.size() > 0 )
	{
		g_theRenderer->DrawVertexArray( healthSegments );
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
void Entity::SetValuesFromXML( XmlElement const& element )
{
	const float	invalidYaw = ~0;
	const Vec2	invalidPos = Vec2( -1.f, -1.f );

	Vec2 entityPos = ParseXmlAttribute( element, "pos", invalidPos );
	if( entityPos == invalidPos )
	{
		g_theConsole->ErrorString( "Failed to parse \"pos\" of entity at line %i", element.GetLineNum() );
		g_theConsole->ErrorString( "Position set to ( 1, 1 )" );
		entityPos = Vec2( 1.f, 1.f );
	}

	float entityYaw = ParseXmlAttribute( element, "yaw", invalidYaw );
	if( entityYaw == invalidYaw )
	{
		g_theConsole->ErrorString( "Failed to parse \"yaw\" of entity at line %i", element.GetLineNum() );
		g_theConsole->ErrorString( "Yaw set to 0 degrees" );
		entityYaw = 0.f;
	}

	SetPosition( Vec3( entityPos, 0.f ) );
	SetYaw( entityYaw );
}


//---------------------------------------------------------------------------------------------------------
void Entity::SetValuesFromEntityData( EntityData const& entityData )
{
	m_isPossessed			= entityData.m_isPossessed;
	m_canBePushedByWalls	= entityData.m_canBePushedByWalls;
	m_canBePushedByEntities	= entityData.m_canBePushedByEntities;
	m_canPushEntities		= entityData.m_canPushEntities;
	m_mass					= entityData.m_mass;
	m_isDead				= entityData.m_isDead;
	m_currentHealth			= entityData.m_currentHealth;

	m_position			= entityData.m_position;
	m_forwardDirection	= entityData.m_forwardDirection;
	m_yaw				= entityData.m_yaw;
	m_actionState		= entityData.m_actionState;
}

//---------------------------------------------------------------------------------------------------------
std::string Entity::GetEntityName() const
{
	return m_entityDef.GetName();
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
EntityData Entity::GetEntityData() const
{
	EntityData entityData;
	entityData.m_isPossessed			= m_isPossessed;
	entityData.m_canBePushedByWalls		= m_canBePushedByWalls;
	entityData.m_canBePushedByEntities	= m_canBePushedByEntities;
	entityData.m_canPushEntities		= m_canPushEntities;
	entityData.m_mass					= m_mass;
	entityData.m_isDead					= m_isDead;
	entityData.m_currentHealth			= m_currentHealth;

	entityData.m_position = m_position;
	entityData.m_forwardDirection = m_forwardDirection;
	entityData.m_yaw				= m_yaw;
	memcpy( &entityData.m_actionState[0], &m_actionState[0], m_actionState.size() );
	std::string entityDefName = m_entityDef.GetName();
	memcpy( &entityData.m_entityDefName[0], &entityDefName[0], entityDefName.size() );

	return entityData;
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
void Entity::SetForward( Vec2 const& forward )
{
	m_forwardDirection = forward;
}


//---------------------------------------------------------------------------------------------------------
void Entity::SetYaw( float yaw )
{
	m_yaw = yaw;
}


//---------------------------------------------------------------------------------------------------------
void Entity::AddYaw( float yawToAdd )
{
	m_yaw += yawToAdd;
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


//---------------------------------------------------------------------------------------------------------
void Entity::SetMap( Map* map )
{
	m_theMap = map;
}


//---------------------------------------------------------------------------------------------------------
void Entity::TakeDamage( int damageToTake )
{
	m_currentHealth -= damageToTake;
	if( m_currentHealth <= 0 )
	{
		m_isDead = true;
		g_theApp->SendReliableUpdate();
	}
}


//---------------------------------------------------------------------------------------------------------
void Entity::Shoot()
{
	EntityDef* bulletDef = EntityDef::GetEntityDefByName( "Plasma Bolt" );
	Projectile* newBullet = new Projectile( m_theGame, m_theWorld, m_theMap, *bulletDef );
	newBullet->SetPosition( m_position + Vec3( 0.f, 0.f, GetEyeHeight() ) + ( Vec3( m_forwardDirection, 0.f ) * ( GetPhysicsRadius() + newBullet->GetPhysicsRadius() + 0.01f ) ) );
	newBullet->SetForward( m_forwardDirection );
	m_theMap->AddEntityToMap( newBullet );

	g_theApp->SendReliableUpdate();
}
