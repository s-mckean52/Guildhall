#pragma once
#include "Engine/Core/EventSubscription.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include <string>
#include <vector>

template<typename ...ARGS>
class Delegate;

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

	template<typename OBJ_TYPE>
	void SubscribeEventCallbackMethod( std::string eventName, OBJ_TYPE* object, void( OBJ_TYPE::*methodCallback )( EventArgs* args ) );
	template<typename OBJ_TYPE>
	void UnsubscribeEventCallbackMethod( std::string eventName, OBJ_TYPE* object, void( OBJ_TYPE::*methodCallback )( EventArgs* args ) );
	template<typename OBJ_TYPE>
	void UnsubscribeObjectFromEvents( OBJ_TYPE* object );


private:
	std::vector< EventSubscription* > m_eventSubscriptions;
};


//---------------------------------------------------------------------------------------------------------
template<typename OBJ_TYPE>
void EventSystem::SubscribeEventCallbackMethod( std::string eventName, OBJ_TYPE* object, void( OBJ_TYPE::*methodCallback )( EventArgs* args ) )
{
	for( uint eventSubIndex = 0; eventSubIndex < m_eventSubscriptions.size(); ++eventSubIndex )
	{
		EventSubscription* currentEventSub = m_eventSubscriptions[ eventSubIndex ];
		
		if( currentEventSub == nullptr ) 
			continue;

		if( eventName == currentEventSub->GetEventName() )
		{
			currentEventSub->SubscribeMethodCallbackToDelegate( object, methodCallback );
			return;
		}
	}
	EventSubscription* newEventSubscription = new EventSubscription( eventName, object, methodCallback );
	m_eventSubscriptions.push_back( newEventSubscription );
}


//---------------------------------------------------------------------------------------------------------
template<typename OBJ_TYPE>
void EventSystem::UnsubscribeEventCallbackMethod( std::string eventName, OBJ_TYPE* object, void( OBJ_TYPE::*methodCallback )( EventArgs* args ) )
{
	for( int eventSubIndex = 0; eventSubIndex < m_eventSubscriptions.size(); ++eventSubIndex )
	{
		EventSubscription* currentEventSub = m_eventSubscriptions[ eventSubIndex ];

		if( currentEventSub == nullptr )
			continue;

		if( currentEventSub->m_eventName == eventName )
		{
			currentEventSub->UnsubscribeMethodCallbackToDelegate( object, methodCallback );
			if( currentEventSub->IsEmpty() )
			{
				delete currentEventSub;
				m_eventSubscriptions[ eventSubIndex ] = m_eventSubscriptions[ m_eventSubscriptions.size() - 1 ];
				m_eventSubscriptions.pop_back();
			}
			break; //only removes one EventSub
		}
	}
}


//---------------------------------------------------------------------------------------------------------
template<typename OBJ_TYPE>
void EventSystem::UnsubscribeObjectFromEvents( OBJ_TYPE* object )
{
	for( uint eventSubIndex = 0; eventSubIndex < m_eventSubscriptions.size(); ++eventSubIndex )
	{
		m_eventSubscriptions[ eventSubIndex ]->m_callbackDelegate.UnsubscribeObject( object );
	}
}
