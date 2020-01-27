#pragma once

#include <string.h>

class InputSystem;

class Window
{
public:
	Window();
	~Window();

	bool Open( std::string const& title, float clientAspect = 16.f / 9.f, float ratioOfHeight = 0.9f );
	void Close();

	void SetInputSystem( InputSystem* input );
	InputSystem* GetInputSystem()	const { return m_theInput; }

	void BeginFrame();

	unsigned int GetClientWidth()	const { return m_width; }
	unsigned int GetClientHeight()	const { return m_height; }

public:
	void* m_hwnd;
	unsigned int m_height	= 0;
	unsigned int m_width	= 0;
	InputSystem* m_theInput = nullptr;
};