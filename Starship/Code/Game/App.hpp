#pragma once

class App
{
public:
	bool m_isSlowMo = false;
	bool m_isPaused = false;
	bool m_isQuitting = false;

public:
	App() {};
	~App() {};
	void StartUp();
	void ShutDown();
	void RunFrame();
	void RestartGame();

	bool HandleKeyPressed( unsigned char keyCode );
	bool HandleKeyReleased( unsigned char keyCode );
	bool HandleQuitRequested();

	const bool IsQuitting() const { return m_isQuitting; }

private:
	void BeginFrame();
	void Update( float deltaSeconds );
	void Render() const;
	void EndFrame();
};