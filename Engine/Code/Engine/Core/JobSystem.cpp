#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"

//---------------------------------------------------------------------------------------------------------
std::atomic<bool> g_isJobSystemQuitting = false;


//---------------------------------------------------------------------------------------------------------
//
// Job
//
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
Job::Job()
{
	static int s_nextJobID = 1;
	m_jobID = s_nextJobID++;
}


//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//
// Worker Thread
//
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
WorkerThread::WorkerThread()
{
	m_threadedObject = new std::thread( &WorkerThread::WorkerThreadMain, this );
}


//---------------------------------------------------------------------------------------------------------
WorkerThread::~WorkerThread()
{
	m_threadedObject->join();

	delete m_threadedObject;
}


//---------------------------------------------------------------------------------------------------------
void WorkerThread::WorkerThreadMain()
{
	while ( !g_isJobSystemQuitting )
	{
		Job* job = g_theJobSystem->GetBestAvailableJob();
		if( job != nullptr )
		{
			job->Execute();
			g_theJobSystem->OnJobCompleted( job );
		}
		else
		{
			std::this_thread::sleep_for( std::chrono::microseconds(10) );
		}
	}
}


//---------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
//
// Job System
//
//---------------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------------
void JobSystem::ShutDown()
{
	// Order here is important to ensure we grab all threads and jobs
	g_isJobSystemQuitting = true;

	DeleteWorkerThreads();
	DeleteQueuedJobs();
	DeleteCompletedJobs();
}


//---------------------------------------------------------------------------------------------------------
void JobSystem::DeleteWorkerThreads()
{
	for ( int workerThreadIndex = 0; workerThreadIndex < m_workerThreads.size(); ++workerThreadIndex )
	{
		delete m_workerThreads[ workerThreadIndex ];
		m_workerThreads[ workerThreadIndex ] = nullptr;
	}
}


//---------------------------------------------------------------------------------------------------------
void JobSystem::DeleteQueuedJobs()
{
	m_jobsQueuedMutex.lock();

	for( auto jobIndexIter = m_jobsQueued.begin(); jobIndexIter != m_jobsQueued.end(); ++jobIndexIter )
	{		
		delete *jobIndexIter;
		*jobIndexIter = nullptr;
	}

	m_jobsQueuedMutex.unlock();
}


//---------------------------------------------------------------------------------------------------------
void JobSystem::DeleteCompletedJobs()
{
	m_jobsCompletedMutex.lock();

	for( auto jobIndexIter = m_jobsCompleted.begin(); jobIndexIter != m_jobsCompleted.end(); ++jobIndexIter )
	{		
		delete *jobIndexIter;
		*jobIndexIter = nullptr;
	}

	m_jobsCompletedMutex.unlock();
}


//---------------------------------------------------------------------------------------------------------
void JobSystem::CreateWorkerThreads( int numThreadsToCreate )
{
	for( int threadCount = 0; threadCount < numThreadsToCreate; ++threadCount )
	{
		CreateWorkerThread();
	}
}


//---------------------------------------------------------------------------------------------------------
void JobSystem::CreateWorkerThread()
{
	WorkerThread* workerThread = new WorkerThread();
	
	m_workerThreads.push_back( workerThread );
}


//---------------------------------------------------------------------------------------------------------
void JobSystem::PostJob( Job* job )
{
	GUARANTEE_OR_DIE( job != nullptr, "Cannot add add job nullptr to list" );

	m_jobsQueuedMutex.lock();

	m_jobsQueued.push_back( job );

	m_jobsQueuedMutex.unlock();
}


//---------------------------------------------------------------------------------------------------------
void JobSystem::OnJobCompleted( Job* job )
{
	m_jobsRunningMutex.lock();
	m_jobsCompletedMutex.lock();
	
	for( auto jobIter = m_jobsRunning.begin(); jobIter != m_jobsRunning.end(); ++jobIter )
	{
		Job* runningJob = *jobIter;
		if( runningJob == job )
		{
			m_jobsCompleted.push_back( job );
			m_jobsRunning.erase( jobIter );
			break;
		}
	}

	m_jobsCompletedMutex.unlock();
	m_jobsRunningMutex.unlock();
}


//---------------------------------------------------------------------------------------------------------
void JobSystem::ClaimAndDeleteAllCompletedJobs()
{
	std::deque<Job*> claimedJobs;

	m_jobsCompletedMutex.lock();
	m_jobsCompleted.swap( claimedJobs );
	m_jobsCompletedMutex.unlock();

	for( auto claimedJobIter = claimedJobs.begin(); claimedJobIter != claimedJobs.end(); ++claimedJobIter )
	{
		Job* job = *claimedJobIter;
		job->OnCompleteCallback();
		delete job;
	}
}


//---------------------------------------------------------------------------------------------------------
Job* JobSystem::GetBestAvailableJob()
{
	Job* job = nullptr;

	m_jobsQueuedMutex.lock();

	if( m_jobsQueued.size() > 0 )
	{
		job = m_jobsQueued.front();
		m_jobsQueued.pop_front();

		m_jobsRunningMutex.lock();
		m_jobsRunning.push_back( job );
		m_jobsRunningMutex.unlock();
	}

	m_jobsQueuedMutex.unlock();

	return job;
}


//---------------------------------------------------------------------------------------------------------
void JobSystem::WaitForAllJobs()
{
	bool isWaitingForJobs = true;
	while( isWaitingForJobs )
	{
		m_jobsQueuedMutex.lock();
		if( m_jobsQueued.size() <= 0 )
		{
			isWaitingForJobs = false;
		}
		m_jobsQueuedMutex.unlock();
		std::this_thread::sleep_for( std::chrono::microseconds(10) );
	}
}

