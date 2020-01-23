#pragma once

#include <string.h>

class Window
{
public:
	Window();
	~Window();

	bool Open( std::string const& title, float clientAspect = 16.f / 9.f, float ratioOfHeight = 0.9f );
	void Close();

	void BeginFrame();

public:
	void* m_hwnd;

};