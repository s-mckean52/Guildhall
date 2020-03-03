#pragma once
#include <vector>


class Clock
{
public:
	static Clock s_masterClock; 

public:
	Clock();
	Clock( Clock* parent );
	~Clock();

	void Update( double deltaSeconds );
	void Reset();

	void Pause();
	void Resume();
	void SetScale( double scale );

	void SetFrameLimits( double minFrameTime, double maxFrameTime );

public:
	void	SetParent( Clock* newParent );
	void	AddChild( Clock* newChild );
	void	RemoveChild( Clock* childToRemove );

	bool	HasParent() const;
	bool	IsPaused() const				{ return m_isPaused; } 
	double	GetScale() const				{ return m_scale; }
	double	GetTotalElapsedSeconds() const	{ return m_totalTimeSeconds; }
	double	GetLastDeltaSeconds() const		{ return m_lastDeltaSeconds; }

public:
	static void SystemStartUp();
	static void SystemShutdown();
	static void BeginFrame();

	static Clock* GetMaster();

private:
	double m_scale = 1.f;
	double m_totalTimeSeconds = 0.f;
	double m_lastDeltaSeconds = 0.f;

	double m_minFrameTime = 0.0;
	double m_maxFrameTime = 0.1;
	
	bool m_isPaused = false;

	Clock* m_parentClock = nullptr;
	std::vector<Clock*> m_childrenClocks;
};