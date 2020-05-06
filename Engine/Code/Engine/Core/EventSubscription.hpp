#pragma once
#include "Engine/Core/Delegate.hpp"
#include <string>

template<typename ...ARGS>
class Delegate;
class NamedProperties;

typedef NamedProperties EventArgs;
typedef void( *EventCallbackFunctionPtrType )( EventArgs* args );


struct EventSubscription
{
	friend class EventSystem;

public:
	explicit EventSubscription() = default;
	explicit EventSubscription( std::string eventName, EventCallbackFunctionPtrType callbackFunction );
	
	template<typename OBJ_TYPE>
	explicit EventSubscription( std::string eventName, OBJ_TYPE* object, void( OBJ_TYPE::*methodCallback )( EventArgs* args ) );

	std::string GetEventName() const { return m_eventName; }
	bool		IsEmpty() const;

private:
	void SubscribeFunctionCallbackToDelegate( EventCallbackFunctionPtrType callbackFunction );
	void UnsubscribeFunctionCallbackToDelegate( EventCallbackFunctionPtrType callbackFunction );

	template<typename OBJ_TYPE>
	void SubscribeMethodCallbackToDelegate( OBJ_TYPE* object, void( OBJ_TYPE::*methodCallback )( EventArgs* args ) );

	template<typename OBJ_TYPE>
	void UnsubscribeMethodCallbackToDelegate( OBJ_TYPE* object, void( OBJ_TYPE::*methodCallback )( EventArgs* args ) );

private:
	std::string m_eventName = "";
	Delegate<EventArgs*> m_callbackDelegate;
	//EventCallbackFunctionPtrType m_callbackFunction = nullptr;
};


//---------------------------------------------------------------------------------------------------------
template<typename OBJ_TYPE>
EventSubscription::EventSubscription( std::string eventName, OBJ_TYPE* object, void( OBJ_TYPE::*methodCallback )( EventArgs* args ) )
{
	m_eventName = eventName;
	m_callbackDelegate.SubscribeMethod( object, methodCallback );
}


//---------------------------------------------------------------------------------------------------------
template<typename OBJ_TYPE>
void EventSubscription::SubscribeMethodCallbackToDelegate( OBJ_TYPE* object, void( OBJ_TYPE::*methodCallback )( EventArgs* args ) )
{
	m_callbackDelegate.SubscribeMethod( object, methodCallback );
}


//---------------------------------------------------------------------------------------------------------
template<typename OBJ_TYPE>
void EventSubscription::UnsubscribeMethodCallbackToDelegate( OBJ_TYPE* object, void( OBJ_TYPE::*methodCallback )( EventArgs* args ) )
{
	m_callbackDelegate.UnsubscribeMethod( object, methodCallback );
}

