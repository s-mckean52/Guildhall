#pragma once
#include "Engine/Core/EventSubscription.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <string>
#include <vector>

struct EventSubscription;

typedef unsigned int EntityID;
typedef std::vector< std::string > Strings;

class EventSystem
{
public:
	void StartUp();
	void ShutDown();
	void StartFrame();
	void EndFrame();

	Strings GetEventNames();
	void SubscribeEventCallbackFunction( std::string eventName, EventCallbackFunctionPtrType functionToCall ); 
	void UnsubscribeEventCallbackFunction( std::string eventName, EventCallbackFunctionPtrType functionToCall ); 
	void FireEvent( std::string eventToFire, std::string eventArguments = "" );

private:
	std::vector< EventSubscription* > m_eventSubscriptions;
};