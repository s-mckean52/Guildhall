#pragma once
#include <vector>
#include <functional>

template <typename ...ARGS> 
class Delegate
{
public:
	using c_callback_t = void (*) ( ARGS... );
	using function_t = std::function< void( ARGS... ) >;

	struct subscription
	{
		void const* objectID = nullptr;
		void const* functionID = nullptr;
		function_t callable;
	
		inline bool operator== ( subscription const& other ) const		{ return ( objectID == other.objectID ) && ( functionID == other.functionID ); }
	};


public:
	void Subscribe( c_callback_t const& callback );
	void Unsubscribe( c_callback_t const& callback );

	template<typename OBJ_TYPE>
	void SubscribeMethod( OBJ_TYPE* obj, void( OBJ_TYPE::*methodCallback )( ARGS... ) );
	template<typename OBJ_TYPE>
	void UnsubscribeMethod( OBJ_TYPE* obj, void( OBJ_TYPE::*methodCallback )( ARGS... ) );
	template<typename OBJ_TYPE>
	void UnsubscribeObject( OBJ_TYPE* obj );

	void Invoke( ARGS const& ...args );

private:
	void Subscribe( subscription const& sub );
	void Unsubscribe( subscription const& sub );

public:
	std::vector<subscription> m_subscribers;

	void operator() ( ARGS const& ...args ) { Invoke( args... ); }
};


//---------------------------------------------------------------------------------------------------------
template<typename ...ARGS>
template<typename OBJ_TYPE>
void Delegate<ARGS...>::UnsubscribeObject( OBJ_TYPE* obj )
{
	for( int subIndex = 0; subIndex < m_subscribers.size(); )
	{
		if( m_subscribers[subIndex].objectID == obj )
		{
			m_subscribers.erase( m_subscribers.begin() + subIndex );
			continue;
		}
		++subIndex;
	}
}

template<typename ...ARGS>
template<typename OBJ_TYPE>
void Delegate< ARGS... >::UnsubscribeMethod( OBJ_TYPE* obj, void( OBJ_TYPE::*methodCallback )( ARGS... ) )
{
	subscription sub;
	sub.objectID = obj;
	sub.functionID = *( void const** ) &methodCallback;

	Unsubscribe( sub );
}

template<typename ...ARGS>
template<typename OBJ_TYPE>
void Delegate< ARGS... >::SubscribeMethod( OBJ_TYPE* obj, void( OBJ_TYPE::*methodCallback )( ARGS... ) )
{
	subscription sub;
	sub.objectID = obj;
	sub.functionID = *(void const**) &methodCallback;

	sub.callable = [=]( ARGS ...args ) { ( obj->*methodCallback )( args... ); };

	Subscribe( sub );
}

//---------------------------------------------------------------------------------------------------------
template <typename ...ARGS>
void Delegate<ARGS...>::Unsubscribe( subscription const& sub )
{
	for( unsigned int subIndex = 0; subIndex < m_subscribers.size(); ++subIndex )
	{
		if( m_subscribers[subIndex] == sub )
		{
			m_subscribers.erase( m_subscribers.begin() + subIndex );
			return;
		}
	}
}


//---------------------------------------------------------------------------------------------------------
template <typename ...ARGS>
void Delegate<ARGS...>::Unsubscribe( c_callback_t const& callback )
{
	subscription sub;
	sub.objectID = nullptr;
	sub.functionID = callback;

	Unsubscribe( sub );
}


//---------------------------------------------------------------------------------------------------------
template <typename ...ARGS>
void Delegate<ARGS...>::Subscribe( subscription const& sub )
{
	m_subscribers.push_back( sub );
}


//---------------------------------------------------------------------------------------------------------
template<typename ...ARGS>
void Delegate<ARGS...>::Subscribe( c_callback_t const& callback )
{
	subscription sub;

	sub.objectID = nullptr;
	sub.functionID = callback;
	sub.callable = callback;

	Subscribe( sub );
}


//---------------------------------------------------------------------------------------------------------
template<typename ...ARGS>
void Delegate< ARGS... >::Invoke( ARGS const& ...args )
{
	for( subscription& sub : m_subscribers )
	{
		sub.callable( args... );
	}
}
