#pragma once
#include "Engine/Core/NamedStrings.hpp"
#include <string>
#include <vector>

struct EventSubscription;

typedef unsigned int EntityID;
typedef std::vector< std::string > Strings;
typedef void( *EventCallbackFunctionPtrType )( NamedStrings* args );

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