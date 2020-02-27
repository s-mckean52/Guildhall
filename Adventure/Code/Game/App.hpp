#pragma once

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

	bool HandleQuitRequested();
	void OpenDevConsole();

	const bool IsQuitting() const { return m_isQuitting; }

	static void QuitRequested();
	static void HelpCommand();

private:
	void BeginFrame();
	void Update( float deltaSeconds );
	void Render() const;
	void EndFrame();
};