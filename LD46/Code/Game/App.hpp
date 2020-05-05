#pragma once

class NamedStrings;
class Camera;

class App
{
private:
	bool m_isQuitting = false;
	Camera* m_devConsoleCamera = nullptr;

public:
	App() {};
	~App() {};
	void StartUp();
	void ShutDown();
	void RunFrame();
	void RestartGame();

	static void HelpCommand( NamedStrings* args );
	static void QuitRequested( NamedStrings* args );
	bool HandleQuitRequested();

	const bool IsQuitting() const { return m_isQuitting; }

private:
	void BeginFrame();
	void Update();
	void Render() const;
	void EndFrame();
};