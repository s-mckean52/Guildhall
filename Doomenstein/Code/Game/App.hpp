#pragma once
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"

class Camera;
class Server;

class App
{
private:
	bool	m_isQuitting		= false;
	Server*	m_theServer			= nullptr;
	Camera*	m_devConsoleCamera	= nullptr;

public:
	App() {};
	~App() {};
	void StartUp();
	void ShutDown();
	void RunFrame();
	void RestartGame();

	void start_multiplayer_server( EventArgs* args );
	void connect_to_mulitplayer_server( EventArgs* args );
	void SendReliableUpdate();

	static void HelpCommand( EventArgs* args );
	static void QuitRequested( EventArgs* args );
	bool HandleQuitRequested();

	const bool IsQuitting() const { return m_isQuitting; }

private:
	void BeginFrame();
	void Update();
	void Render() const;
	void EndFrame();
};