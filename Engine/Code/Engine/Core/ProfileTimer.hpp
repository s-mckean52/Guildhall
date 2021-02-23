#pragma once


class ProfileTimer
{
public:
	ProfileTimer( char const* name );
	~ProfileTimer();

	void StartTimer();
	void StopTimer();

	char const* GetName() const						{ return m_name; }
	float GetLastCheckDurationSeconds() const;
	float GetLastCheckDurationMilliseconds() const;
	float GetAvgTimeSeconds() const;
	float GetAvgTimeMilliseconds() const;

private:
	const char*	m_name						= "";
	double		m_startTimeSeconds			= 0.0;
	double		m_endTimeSeconds			= 0.0;
	double		m_lastCheckDurationSeconds	= 0.0;
	double		m_avgTimeSeconds			= 0.0;
	double		m_samples					= 0.0;
};