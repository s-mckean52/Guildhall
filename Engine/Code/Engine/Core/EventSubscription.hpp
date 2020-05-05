#pragma once
#include <string>

class NamedProperties;

typedef NamedProperties EventArgs;
typedef void( *EventCallbackFunctionPtrType )( EventArgs* args );

struct EventSubscription
{
	friend class EventSystem;

public:
	explicit EventSubscription() = default;
	explicit EventSubscription( std::string eventName, EventCallbackFunctionPtrType callbackFunction );

	std::string GetEventName() const { return m_eventName; }

private:
	std::string m_eventName = "";
	EventCallbackFunctionPtrType m_callbackFunction = nullptr;
};