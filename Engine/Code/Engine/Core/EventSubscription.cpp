#include "Engine/Core/EventSubscription.hpp"


//---------------------------------------------------------------------------------------------------------
EventSubscription::EventSubscription( std::string eventName, EventCallbackFunctionPtrType callbackFunction )
	: m_eventName( eventName )
	, m_callbackFunction( callbackFunction )
{
}
