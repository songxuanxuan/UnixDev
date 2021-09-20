#ifndef _HTTP_TIME_MINHEAP_H
#define _HTTP_TIME_MINHEAP_H

#include "http_conn.h"

struct timer_unit {
	timer_unit(int delay) : expire(time(nullptr) + delay) {};
	time_t expire;
	void (*cb_func)(http_conn*);
	void fresh_expire(int delay) {
		expire = time(nullptr) + delay;
	}
	http_conn* conn_data;
};

class timer_heap
{
public:
	timer_heap(int cap) throw(exception);
	timer_heap(timer_unit** init_arr, int size, int cap)  throw(exception);
	~timer_heap();
	//尾部插入,进行上滤
	void add_timer(timer_unit* timer) throw(exception);
	//删除
	void del_timer(timer_unit* timer);
	//获得顶部定时器
	timer_unit* top() const;
	void tick();
	void pop();
	void resize()throw(exception);
	bool empty()const { return cur_size == 0; }
private:
	void percolate(int hole)
	{
		auto tmp = arr[hole];
		int child = hole * 2 + 1;
		while (child < cur_size )
		{
			if (arr[child]->expire > arr[child + 1]->expire)
				++child;
			if (arr[child]->expire < tmp->expire)
				arr[hole] = arr[child];
			else
				break;
			hole = child;
			child = child * 2 + 1;
		}
		arr[hole] = tmp;
	}
	timer_unit** arr;
	size_t capacity;
	size_t cur_size;
};


timer_heap::timer_heap(timer_unit** init_arr, int size, int cap) throw(exception) : timer_heap(cap)
{
	if (capacity < size)
		throw exception();
	cur_size = size;
	if (size != 0)
	{
		//初始化
		for (int i = 0; i < cur_size; ++i)
			arr[i] = init_arr[i];
		//从第一个非叶节点排序
		for (int i = (cur_size - 1) / 2; i >= 0; --i)
			percolate(i);
	}
}

timer_heap::timer_heap(int cap) throw(exception) :capacity(cap), cur_size(0)
{
	arr = new timer_unit * [capacity];
	if (!arr)
		throw exception();
	for (int i = 0;i<cur_size;++i)
		arr[i] = nullptr;
}

timer_heap::~timer_heap()
{
	for (int i = 0; i < cur_size; ++i)
		arr[i] = nullptr;
	delete[] arr;
}

void timer_heap::add_timer(timer_unit* timer) throw(exception)
{
	if (!timer) return;
	if (cur_size >= capacity)
		resize();
	int hole = cur_size++;
	int parent = 0;
	for (; hole > 0; hole = parent)
	{
		parent = (hole - 1) / 2;
		if (arr[parent]->expire <= timer->expire)
			break;
		arr[hole] = arr[parent];
	}
	arr[hole] = timer;
}

inline void timer_heap::del_timer(timer_unit* timer)
{
	if (!timer) return;
	/*仅仅将目标定时器的回调函数设置为空，即所谓的延迟销毁。这将节省真正删除该定时器造成的开销，但这样做容易使堆数组膨胀*/
	timer->cb_func = nullptr;
}

inline timer_unit* timer_heap::top() const
{
	if (empty()) return nullptr;
	return arr[0];
}

void timer_heap::tick()
{
	auto tmp = arr[0];
	time_t cur = time(nullptr);
	while (!empty())
	{
		if (!arr[0]) break;
		if (tmp->expire > cur)
			break;
		if (tmp->cb_func)
			tmp->cb_func(tmp->conn_data);
		pop();
#ifdef DEBUG
		printf("tick done with timeout \n");
		printf("connecting %d users\n", cur_size);
#endif // DEBUG
		tmp = arr[0];
	}
}

void timer_heap::pop()
{
	if (empty()) return;
	if (arr[0])
	{
		delete arr[0];
		arr[0] = arr[--cur_size];
		percolate(0);
	}
}

void timer_heap::resize() throw(exception)
{
	auto tmp = new timer_unit * [2 * capacity];
	if (!tmp)
		throw exception();
	capacity = 2 * capacity;
	for (int i = 0; i < capacity; ++i)
		tmp[i] = nullptr;
	for (int i = 0; i < cur_size; ++i)
		tmp[i] = arr[i];
	delete[] arr;
	arr = tmp;
}

#endif // _HTTP_TIME_MINHEAP_H__
