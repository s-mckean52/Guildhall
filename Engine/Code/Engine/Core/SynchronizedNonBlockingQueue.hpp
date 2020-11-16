#pragma once
#include <deque>
#include <mutex>

template<typename T>
class SynchronizedNonBlockingQueue
{
public:
	SynchronizedNonBlockingQueue() = default;
	~SynchronizedNonBlockingQueue() = default;

	void Swap( std::deque<T>& copyToQueue );
	void Push( T const& elem );
	bool Pop( T& out_value );

private:
	std::mutex		m_queueMutex;
	std::deque<T>	m_queue;
};


//---------------------------------------------------------------------------------------------------------
template<typename T>
void SynchronizedNonBlockingQueue<T>::Swap( std::deque<T>& copyToQueue )
{
	m_queueMutex.lock();
	m_queue.swap( copyToQueue );
	m_queueMutex.unlock();
}

//---------------------------------------------------------------------------------------------------------
template<typename T>
bool SynchronizedNonBlockingQueue<T>::Pop( T& out_value )
{
	bool wasPopSuccessful = false;

	m_queueMutex.lock();

	if( !m_queue.empty() )
	{
		out_value = m_queue.front();
		m_queue.pop_front();
		wasPopSuccessful = true;
	}

	m_queueMutex.unlock();

	return wasPopSuccessful;
}


//---------------------------------------------------------------------------------------------------------
template<typename T>
void SynchronizedNonBlockingQueue<T>::Push( T const& elem )
{
	m_queueMutex.lock();

	m_queue.push_back( elem );

	m_queueMutex.unlock();
}