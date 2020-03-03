#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Math/MathUtils.hpp"


STATIC Clock Clock::s_masterClock( nullptr );


//---------------------------------------------------------------------------------------------------------
Clock::Clock()
{
	SetParent( Clock::GetMaster() );
}


//---------------------------------------------------------------------------------------------------------
Clock::Clock( Clock* parent )
{
	m_parentClock = parent;
	if( parent != nullptr )
	{
		SetParent( parent );
	}
}


//---------------------------------------------------------------------------------------------------------
Clock::~Clock()
{
	for( int childClockIndex = 0; childClockIndex < m_childrenClocks.size(); ++childClockIndex )
	{
		Clock* currentChildClock = m_childrenClocks[ childClockIndex ];
		if( currentChildClock != nullptr )
		{
			m_childrenClocks[ childClockIndex ]->SetParent( m_parentClock );
		}
	}

	if( m_parentClock != nullptr )
	{
		m_parentClock->RemoveChild( this );
	}
}


//---------------------------------------------------------------------------------------------------------
void Clock::Update( double deltaSeconds )
{
	if( IsPaused() )
	{
		deltaSeconds = 0.0;
	}
	else
	{
		Clamp( deltaSeconds, m_minFrameTime, m_maxFrameTime );
		deltaSeconds *= m_scale;
	}

	m_lastDeltaSeconds = deltaSeconds;
	m_totalTimeSeconds += deltaSeconds;

	for( int childrenClockIndex = 0; childrenClockIndex < m_childrenClocks.size(); ++childrenClockIndex )
	{
		Clock* currentChildClock = m_childrenClocks[ childrenClockIndex ];
		if( currentChildClock != nullptr )
		{
			currentChildClock->Update( deltaSeconds );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void Clock::Reset()
{
	m_lastDeltaSeconds = 0.0;
	m_totalTimeSeconds = 0.0;
}


//---------------------------------------------------------------------------------------------------------
void Clock::Pause()
{
	m_isPaused = true;
}


//---------------------------------------------------------------------------------------------------------
void Clock::Resume()
{
	m_isPaused = false;
}


//---------------------------------------------------------------------------------------------------------
void Clock::SetScale( double scale )
{
	m_scale = scale;
}


//---------------------------------------------------------------------------------------------------------
void Clock::SetFrameLimits( double minFrameTime, double maxFrameTime )
{
	m_minFrameTime = minFrameTime;
	m_maxFrameTime = maxFrameTime;
}


//---------------------------------------------------------------------------------------------------------
void Clock::SetParent( Clock* newParent )
{
	if( m_parentClock != nullptr )
	{
		m_parentClock->RemoveChild( this );
	}
	m_parentClock = newParent;
	newParent->AddChild( this );
}


//---------------------------------------------------------------------------------------------------------
void Clock::AddChild( Clock* newChild )
{
	if( newChild->HasParent() && newChild->m_parentClock != this )
	{
		newChild->SetParent( this );
		return;
	}

	for( int childClockIndex = 0; childClockIndex < m_childrenClocks.size(); ++childClockIndex )
	{
		if( m_childrenClocks[ childClockIndex ] == nullptr )
		{
			m_childrenClocks[ childClockIndex ] = newChild;
			return;
		}
	}
	m_childrenClocks.push_back( newChild );
}


//---------------------------------------------------------------------------------------------------------
void Clock::RemoveChild( Clock* childToRemove )
{
	for( int childClockIndex = 0; childClockIndex < m_childrenClocks.size(); ++childClockIndex )
	{
		if( m_childrenClocks[ childClockIndex ] == childToRemove )
		{
			m_childrenClocks[ childClockIndex ] = nullptr;
			return;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
bool Clock::HasParent() const
{
	if( m_parentClock == nullptr )
		return false;

	return true;
}


//---------------------------------------------------------------------------------------------------------
STATIC void Clock::SystemStartUp()
{
	s_masterClock.Reset();
}


//---------------------------------------------------------------------------------------------------------
STATIC void Clock::SystemShutdown()
{
}


//---------------------------------------------------------------------------------------------------------
STATIC void Clock::BeginFrame()
{
	static double timeLastFrameStarted = GetCurrentTimeSeconds();
	double timeThisFrameStarted = GetCurrentTimeSeconds();
	double deltaSeconds = timeThisFrameStarted - timeLastFrameStarted;
	timeLastFrameStarted = timeThisFrameStarted;

	s_masterClock.Update( deltaSeconds );
}


//---------------------------------------------------------------------------------------------------------
STATIC Clock* Clock::GetMaster()
{
	return &s_masterClock;
}
