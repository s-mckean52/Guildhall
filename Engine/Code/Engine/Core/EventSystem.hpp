#pragma once
#include <string>
#include <vector>

typedef unsigned int EntityID;
typedef std::vector< std::string > Strings;
typedef void( *EventCallbackFunctionPtrType )();
//typedef bool( *EventCallbackFunctionPtrType )();

struct EventSubscription;

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
	void FireEvent( std::string eventToFire );

private:
	std::vector< EventSubscription* > m_eventSubscriptions;
};