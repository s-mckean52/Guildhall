#pragma once

class Camera;
class NamedProperties;
typedef NamedProperties EventArgs;

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

	static void HelpCommand( EventArgs* args );
	static void QuitRequested( EventArgs* args );
	bool HandleQuitRequested();

	void OpenDevConsole();

	const bool IsQuitting() const { return m_isQuitting; }

private:
	void BeginFrame();
	void Update();
	void Render() const;
	void EndFrame();
};