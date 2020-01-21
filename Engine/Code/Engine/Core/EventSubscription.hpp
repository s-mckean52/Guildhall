#pragma once
#include <string>
#include "Engine/Core/EventSystem.hpp"

struct EventSubscription
{
	friend class EventSystem;

public:
	explicit EventSubscription() = default;
	explicit EventSubscription( std::string eventName, EventCallbackFunctionPtrType callbackFunction );

private:
	std::string m_eventName = "";
	EventCallbackFunctionPtrType m_callbackFunction = nullptr;
};