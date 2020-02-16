#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"


class DevConsoleGame
{
public:
	~DevConsoleGame();
	DevConsoleGame();

	//Basic Game
	void StartUp();
	void ShutDown();
	void Render() const;
	void Update( float deltaSeconds );

	//Input
	void RenderCurrentInput() const;
	void ProcessInput();
	void AddCharacterToInput( char c );
	void HandleEscapeKey();
	bool HandleKeyPresses();

	void SubmitCommand();

private:
	bool m_isDevConsoleOpen = false;

	std::string m_currentInput = "";
	Camera	m_devConsoleCamera;
};
