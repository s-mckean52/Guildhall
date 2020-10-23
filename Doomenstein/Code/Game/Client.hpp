#pragma once

class Server;

class Client
{
public:
	Client( Server* owner );
	virtual ~Client();

	virtual void BeginFrame()	= 0;
	virtual void EndFrame()		= 0;
	virtual void Update()		= 0;
	virtual void Render()		= 0;

protected:
	Server* m_owner;
};