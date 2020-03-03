#include "Engine/Core/Timer.hpp"
#include "Engine/Core/Clock.hpp"


//---------------------------------------------------------------------------------------------------------
void Timer::SetSeconds( Clock* clock, double timeToWait )
{
	m_clock = clock;
	m_startSeconds = clock->GetTotalElapsedSeconds();
	m_durationSeconds = timeToWait;
}


//---------------------------------------------------------------------------------------------------------
void Timer::SetSeconds( double timeToWait )
{
	if( m_clock == nullptr )
	{
		m_clock = Clock::GetMaster();
	}

	m_startSeconds = m_clock->GetTotalElapsedSeconds();
	m_durationSeconds = timeToWait;
}


//---------------------------------------------------------------------------------------------------------
void Timer::Reset()
{
	m_startSeconds = m_clock->GetTotalElapsedSeconds();
}


//---------------------------------------------------------------------------------------------------------
void Timer::Stop()
{
	m_durationSeconds = -1.0;
}


//---------------------------------------------------------------------------------------------------------
double Timer::GetElapsedSeconds() const
{
	return m_clock->GetTotalElapsedSeconds() - m_startSeconds;
}


//---------------------------------------------------------------------------------------------------------
double Timer::GetSecondsRemaining() const
{
	double endTimeSeconds = m_startSeconds + m_durationSeconds;
	return endTimeSeconds - m_clock->GetTotalElapsedSeconds();
}


//---------------------------------------------------------------------------------------------------------
bool Timer::HasElapsed() const
{
	double currentTime = m_clock->GetTotalElapsedSeconds();
	return ( currentTime > ( m_startSeconds + m_durationSeconds ) );
}


//---------------------------------------------------------------------------------------------------------
bool Timer::CheckAndDecrement()
{
	if( HasElapsed() )
	{
		m_startSeconds += m_durationSeconds;
		return true;
	}
	else
	{
		return false;
	}
}


//---------------------------------------------------------------------------------------------------------
int Timer::CheckAndDecrementAll()
{
	if( HasElapsed() )
	{
		double totalTimePassed = m_clock->GetTotalElapsedSeconds() - m_startSeconds;
		m_startSeconds = m_clock->GetTotalElapsedSeconds();
		return static_cast<int>( totalTimePassed / m_durationSeconds );
	}
	else
	{
		return 0;
	}
}


//---------------------------------------------------------------------------------------------------------
bool Timer::CheckAndReset()
{
	if( HasElapsed() )
	{
		m_startSeconds = m_clock->GetTotalElapsedSeconds();
		return true;
	}
	else
	{
		return false;
	}
}


//---------------------------------------------------------------------------------------------------------
bool Timer::IsRunning() const
{
	return m_durationSeconds < 0.0;
}

