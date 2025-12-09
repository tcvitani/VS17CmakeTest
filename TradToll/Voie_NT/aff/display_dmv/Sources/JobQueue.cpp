#include "JobQueue.h"

#include <QMutexLocker>

JobQueue::JobQueue()
{
	m_fPending = false;
}

JobQueue::~JobQueue()
{
	Job* pJob;
	while(!m_lstJobQueue.empty())
	{
		pJob = m_lstJobQueue.takeFirst();
		if (pJob != nullptr)
		{
			delete pJob;
		}
	}
}

void JobQueue::EnqueueJob(Job* pJob)
{
	if (pJob == nullptr)
		return;

	QMutexLocker lock(&m_mtxQueueLock);
	m_lstJobQueue.append(pJob);

	if (m_fPending)
	{
		return;
	}

	m_fPending = true;

	// Has to be unlocked if Qt decides to just call the slot
	// connected to the newJobNotification signal instead of queueing it
	lock.unlock(); 

	emit newJobNotification();
}

void JobQueue::ResetPending()
{
	QMutexLocker lock(&m_mtxQueueLock);

	m_fPending = false;

	lock.unlock();
}

void JobQueue::AddJob(Job* pJob)
{
	if (pJob == nullptr)
		return;

	QMutexLocker lock(&m_mtxQueueLock);
	m_lstJobQueue.append(pJob);

	if (m_fPending)
	{
		return;
	}

	m_fPending = true;

	lock.unlock();
}

void JobQueue::DeleteJobs()
{
	Job *pJob;

	QMutexLocker lock(&m_mtxQueueLock);
	
	while (!m_lstJobQueue.empty())
	{
		pJob = m_lstJobQueue.takeFirst();
		if (pJob != nullptr)
		{
			delete pJob;
		}
	}

	m_lstJobQueue.clear();
	
	lock.unlock();
}

Job* JobQueue::ConsumeJob()
{
	QMutexLocker lock(&m_mtxQueueLock);

	m_fPending = false;

	if (m_lstJobQueue.empty())
		return nullptr;

	return m_lstJobQueue.takeFirst();
}
