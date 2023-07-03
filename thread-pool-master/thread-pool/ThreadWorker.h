#include "ThreadPool.h"

using namespace std;

class ThreadWorker {
private:
	int m_id;
	ThreadPool* m_pool;
	bool isWorkingFlag;
public:
	ThreadWorker(ThreadPool* pool, const int id);
	void operator()();
};