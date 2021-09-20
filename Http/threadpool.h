﻿#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <pthread.h>
#include "locker.h"

template<class T>
class threadpool
{
public:
	/*参数thread_number是线程池中线程的数量，max_requests是请求队列中最多允许的、等待处理的请求的数量*/
	threadpool(int thread_number = 8, int max_requests = 10000);
	~threadpool();
	/*往请求队列中添加任务*/
	bool append(T* request);
private:
	/*工作线程运行的函数，它不断从工作队列中取出任务并执行之*/
	static void* worker(void* arg);
	void run();
private:
	/*线程池中的线程数*/
	int m_thread_number;
	/*请求队列中允许的最大请求数*/
	int m_max_requests;
	/*描述线程池的数组，其大小为m_thread_number*/
	pthread_t* m_threads;
	/*请求队列*/
	std::list<T*> m_workqueue;
	/*保护请求队列的互斥锁*/
	locker m_queuelocker;
	/*是否有任务需要处理*/
	sem m_queuestat;
	/*是否结束线程*/
	bool m_stop;
	
};

//线程池创建,FIFO执行请求
template<class T>
threadpool<T>::threadpool(int thread_number /*= 8*/, int max_requests /*= 10000*/)
	:m_thread_number(thread_number), m_max_requests(max_requests), m_stop(false), m_threads(nullptr)
{
	if ((thread_number <= 0) || (max_requests <= 0))
		throw std::exception();
	m_threads = new pthread_t[m_thread_number];
	if (!m_threads)
		throw std::exception();
	for (int i = 0; i < thread_number; ++i)
	{
		printf("create the%dth thread\n", i);
		if (pthread_create(m_threads + i, NULL, worker, this) != 0)
		{
			delete[] m_threads;
			throw std::exception();
		}
		if (pthread_detach(m_threads[i]))
		{
			delete[] m_threads;
			throw std::exception();
		}
	}
}

template<class T>
threadpool<T>::~threadpool()
{
	delete[] m_threads; 
	m_stop = true;
}

//主线程插入队列后发送信号
template<class T>
bool threadpool<T>::append(T* request)
{
	m_queuelocker.lock();
	if (m_workqueue.size() > m_max_requests)
	{
		m_queuelocker.unlock();
		return false;
	}
	m_workqueue.push_back(request);
	m_queuelocker.unlock();
	m_queuestat.post();
	return true;
}

template<class T>
void* threadpool<T>::worker(void* arg)
{
	auto pool = (threadpool*)arg;
	pool->run();
	return pool;
}

//子线程等待信号进行操作
template<class T>
void threadpool<T>::run()
{
	while (!m_stop)
	{
		//等待插入完毕的信号
		m_queuestat.wait();
		m_queuelocker.lock();
		if (m_workqueue.empty())
		{
			m_queuelocker.unlock();
			continue;
		}
		T* request = m_workqueue.front();
		m_workqueue.pop_front();
		m_queuelocker.unlock();
		if(!request)
			continue;
		request->process();
	}
}


#endif // _THREADPOOL_H__
