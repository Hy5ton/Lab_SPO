#include "ThreadWorker.h"
#include <iostream>

ThreadWorker::ThreadWorker(ThreadPool* pool, const int id) : m_pool(pool), m_id(id) 
{
	isWorkingFlag = true;
}

void ThreadWorker::operator()()
{
	function<void()> task;
	bool dequeued;
	while (!m_pool->getShutdownFlag() && isWorkingFlag) {
		{
			unique_lock<mutex> lock(m_pool->getConditionalMutex());
			// cout << "This thread id: " << _threadid << endl;
			if (m_pool->getDeleteFlag()) {
				isWorkingFlag = false;
				m_pool->setDeleteThreadId(m_id);
				m_pool->setDeleteFlag(false);
			}

			if (m_pool->getLock()) {
				m_pool->getConditionalLock().wait(lock);
			}

			//if (m_pool->getQueue().empty()) {
			//	m_pool->getConditionalLock().wait(lock);
			//}

		
		}

		dequeued = m_pool->getQueue().dequeue(task);

		if (dequeued) {
			task();
		}
	}
	cout << "========================\n";
	cout << "Deleting thread: " << _threadid << "\n";
	cout << "========================\n";
	m_pool->removeThread();
}
