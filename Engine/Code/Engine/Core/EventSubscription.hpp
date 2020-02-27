#pragma once
#include "Engine/Core/EventSystem.hpp"
#include <string>

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