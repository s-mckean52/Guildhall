#pragma once

class Clock;


class Timer
{
public:
	void SetSeconds( Clock* clock, double timeToWait );
	void SetSeconds( double timeToWait );

	void Reset();
	void Stop();

	double GetElapsedSeconds() const;
	double GetSecondsRemaining() const;

	bool HasElapsed() const;
	bool CheckAndDecrement();
	int  CheckAndDecrementAll();
	bool CheckAndReset();

	bool IsRunning() const;

public:
	Clock* m_clock;

	double m_startSeconds    =  0.0;
	double m_durationSeconds = -1.0;
};