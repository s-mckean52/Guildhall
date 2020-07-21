#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include <mutex>
#include <atomic>
#include <vector>
#include <deque>
#include <thread>


//---------------------------------------------------------------------------------------------------------
extern std::atomic<bool> g_isJobSystemQuitting;


//---------------------------------------------------------------------------------------------------------
class Job
{
public:
	Job();
	virtual ~Job() {}
	virtual void Execute() = 0;
	virtual void OnCompleteCallback() = 0;

protected:
	int		m_jobID = 0;
};


//---------------------------------------------------------------------------------------------------------
class WorkerThread
{
public:
	WorkerThread();
	~WorkerThread();

	void WorkerThreadMain();

private:
	std::thread*	m_threadedObject = nullptr;
};


//---------------------------------------------------------------------------------------------------------
class JobSystem
{
public:
	JobSystem() {};
	~JobSystem() {};

	void ShutDown();

	void DeleteWorkerThreads();
	void DeleteQueuedJobs();
	void DeleteCompletedJobs();

	void CreateWorkerThreads( int numThreadsToCreate );
	void CreateWorkerThread();
	
	void PostJob( Job* job );
	void OnJobCompleted( Job* job );
	Job* GetBestAvailableJob();
	void ClaimAndDeleteAllCompletedJobs();

private:
	std::deque< Job* >	m_jobsQueued;
	std::deque< Job* >	m_jobsRunning;
	std::deque< Job* >	m_jobsCompleted;
	std::mutex			m_jobsQueuedMutex;
	std::mutex			m_jobsRunningMutex;
	std::mutex			m_jobsCompletedMutex;

	std::vector< WorkerThread* > m_workerThreads;
};