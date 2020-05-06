#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedProperties.hpp"


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
	for( uint eventSubIndex = 0; eventSubIndex < m_eventSubscriptions.size(); ++eventSubIndex )
	{
		EventSubscription* currentEventSub = m_eventSubscriptions[ eventSubIndex ];
		
		if( currentEventSub == nullptr ) 
			continue;

		if( eventName == currentEventSub->GetEventName() )
		{
			currentEventSub->SubscribeFunctionCallbackToDelegate( functionToCall );
			return;
		}
	}

	EventSubscription* newEventSubscription = new EventSubscription( eventName, functionToCall );
	m_eventSubscriptions.push_back( newEventSubscription );
}


//---------------------------------------------------------------------------------------------------------
void EventSystem::UnsubscribeEventCallbackFunction( std::string eventName, EventCallbackFunctionPtrType functionToCall )
{
	for( int eventSubIndex = 0; eventSubIndex < m_eventSubscriptions.size(); ++eventSubIndex )
	{
		EventSubscription* currentEventSub = m_eventSubscriptions[ eventSubIndex ];

		if( currentEventSub == nullptr )
			continue;

		if( currentEventSub->m_eventName == eventName )
		{
			currentEventSub->UnsubscribeFunctionCallbackToDelegate( functionToCall );
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
void EventSystem::FireEvent( std::string eventToFire, std::string eventArguments )
{
	for( int eventSubIndex = 0; eventSubIndex < m_eventSubscriptions.size(); ++eventSubIndex )
	{
		EventSubscription* currentEventSub = m_eventSubscriptions[ eventSubIndex ];

		if( currentEventSub == nullptr )
			continue;

		if( currentEventSub->m_eventName == eventToFire )
		{
			EventArgs args;
			if( eventArguments != "" )
			{
				args.PopulateFromString( eventArguments );
			}
			currentEventSub->m_callbackDelegate( &args );
		}
	}
}
