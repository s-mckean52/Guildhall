#pragma once

class NamedStrings;

class App
{
private:
	bool m_isQuitting = false;

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

	void OpenDevConsole();

	const bool IsQuitting() const { return m_isQuitting; }

private:
	void BeginFrame();
	void Update();
	void Render() const;
	void EndFrame();
};