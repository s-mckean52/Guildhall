#include "Engine/Core/EventSubscription.hpp"


//---------------------------------------------------------------------------------------------------------
EventSubscription::EventSubscription( std::string eventName, EventCallbackFunctionPtrType callbackFunction )
	: m_eventName( eventName )
	//, m_callbackFunction( callbackFunction )
{
	m_callbackDelegate.Subscribe( callbackFunction );
}


//---------------------------------------------------------------------------------------------------------
bool EventSubscription::IsEmpty() const
{
	size_t delegateSize = m_callbackDelegate.m_subscribers.size();
	return ( delegateSize == 0 );
}


//---------------------------------------------------------------------------------------------------------
void EventSubscription::SubscribeFunctionCallbackToDelegate( EventCallbackFunctionPtrType callbackFunction )
{
	m_callbackDelegate.Subscribe( callbackFunction );
}


//---------------------------------------------------------------------------------------------------------
void EventSubscription::UnsubscribeFunctionCallbackToDelegate( EventCallbackFunctionPtrType callbackFunction )
{
	m_callbackDelegate.Unsubscribe( callbackFunction );
}
