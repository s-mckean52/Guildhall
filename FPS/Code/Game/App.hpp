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

	static void HelpCommand();
	static void QuitRequested();
	bool HandleQuitRequested();

	void OpenDevConsole();

	const bool IsQuitting() const { return m_isQuitting; }

private:
	void BeginFrame();
	void Update( float deltaSeconds );
	void Render() const;
	void EndFrame();
};