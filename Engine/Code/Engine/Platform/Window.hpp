#pragma once
#include <string.h>

class InputSystem;
class EventSystem;
struct AABB2;

class Window
{
public:
	Window();
	~Window();

	bool Open( std::string const& title, float clientAspect = 16.f / 9.f, float ratioOfHeight = 0.9f );
	void Close();
	void BeginFrame();

	void SetInputSystem( InputSystem* input );
	void SetEventSystem( EventSystem* eventSystem );
	void SetIsQuitting( bool isQuitting );

	bool			IsQuitting()		const { return m_isQuitting; }
	InputSystem*	GetInputSystem()	const { return m_theInput; }
	EventSystem*	GetEventSystem()	const { return m_theEventSystem; }
	unsigned int	GetClientWidth()	const { return m_width; }
	unsigned int	GetClientHeight()	const { return m_height; }
	Vec2			GetClientCenter()	const;
	AABB2			GetBoundsAsAABB2()	const;

public:
	void* m_hwnd;
	unsigned int m_height	= 0;
	unsigned int m_width	= 0;
	bool m_isQuitting		= false;

	InputSystem* m_theInput			= nullptr;
	EventSystem* m_theEventSystem	= nullptr;
};