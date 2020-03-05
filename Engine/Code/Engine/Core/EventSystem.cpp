#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSubscription.hpp"
#include "Engine/Core/EventArgs.hpp"


//---------------------------------------------------------------------------------------------------------
void EventSystem::StartUp()
{

}


//---------------------------------------------------------------------------------------------------------
void EventSystem::ShutDown()
{

}


//---------------------------------------------------------------------------------------------------------
void EventSystem::StartFrame()
{

}


//---------------------------------------------------------------------------------------------------------
void EventSystem::EndFrame()
{

}


//---------------------------------------------------------------------------------------------------------
Strings EventSystem::GetEventNames()
{
	Strings eventNames;

	for( int eventIndex = 0; eventIndex < m_eventSubscriptions.size(); ++eventIndex )
	{
		std::string eventName = m_eventSubscriptions[ eventIndex ]->GetEventName();
		eventNames.push_back( eventName );
	}

	return eventNames;
}


//---------------------------------------------------------------------------------------------------------
void EventSystem::SubscribeEventCallbackFunction( std::string eventName, EventCallbackFunctionPtrType functionToCall )
{
	EventSubscription* newEventSubscription = new EventSubscription( eventName, functionToCall );
	m_eventSubscriptions.push_back( newEventSubscription );
}


//---------------------------------------------------------------------------------------------------------
void EventSystem::UnsubscribeEventCallbackFunction( std::string eventName, EventCallbackFunctionPtrType functionToCall )
{
	for( int eventSubIndex = 0; eventSubIndex < m_eventSubscriptions.size(); ++eventSubIndex )
	{
		EventSubscription* currentEventSub = m_eventSubscriptions[ eventSubIndex ];

		if( !currentEventSub )
			continue;

		if( currentEventSub->m_eventName == eventName && currentEventSub->m_callbackFunction == functionToCall )
		{
			delete currentEventSub;
			m_eventSubscriptions[ eventSubIndex ] = nullptr;
			break; //only removes one EventSub
		}
	}
}


//---------------------------------------------------------------------------------------------------------
void EventSystem::FireEvent( std::string eventToFire )
{
	for( int eventSubIndex = 0; eventSubIndex < m_eventSubscriptions.size(); ++eventSubIndex )
	{
		EventSubscription* currentEventSub = m_eventSubscriptions[ eventSubIndex ];

		if( !currentEventSub || !currentEventSub->m_callbackFunction )
			continue;

		if( currentEventSub->m_eventName == eventToFire )
		{
			currentEventSub->m_callbackFunction();
		}
	}
}
