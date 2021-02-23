#include "Engine/Core/ProfileTimer.hpp"
#include "Engine/Core/Time.hpp"


//---------------------------------------------------------------------------------------------------------
ProfileTimer::ProfileTimer( char const* name )
	: m_name( name )
{
}


//---------------------------------------------------------------------------------------------------------
ProfileTimer::~ProfileTimer()
{
}


//---------------------------------------------------------------------------------------------------------
void ProfileTimer::StartTimer()
{
	m_startTimeSeconds = GetCurrentTimeSeconds();
}


//---------------------------------------------------------------------------------------------------------
void ProfileTimer::StopTimer()
{
	m_endTimeSeconds = GetCurrentTimeSeconds();
	m_lastCheckDurationSeconds = m_endTimeSeconds - m_startTimeSeconds;

	double currentTotal = m_avgTimeSeconds * m_samples;
	m_samples++;
	m_avgTimeSeconds = ( currentTotal + m_lastCheckDurationSeconds ) / m_samples; 
}


//---------------------------------------------------------------------------------------------------------
float ProfileTimer::GetLastCheckDurationSeconds() const
{
	return static_cast<float>( m_lastCheckDurationSeconds );
}


//---------------------------------------------------------------------------------------------------------
float ProfileTimer::GetLastCheckDurationMilliseconds() const
{
	double lastCheckAsMS = m_lastCheckDurationSeconds * 1000.0;
	return static_cast<float>( lastCheckAsMS );
}


//---------------------------------------------------------------------------------------------------------
float ProfileTimer::GetAvgTimeSeconds() const
{
	return static_cast<float>( m_avgTimeSeconds );
}


//---------------------------------------------------------------------------------------------------------
float ProfileTimer::GetAvgTimeMilliseconds() const
{
	double avgTimeAsMS = m_avgTimeSeconds * 1000.0;
	return static_cast<float>( avgTimeAsMS );
}


