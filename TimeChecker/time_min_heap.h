
/*小根堆存储定时器*/
#ifndef _TIME_MIN_HEAP_H
#define _TIME_MIN_HEAP_H

#include <iostream>
#include <netinet/in.h>
#include <ctime>
using std::exception;
#define BUFFER_SIZE 64
class heap_timer;
struct client_data
{
	sockaddr_in addr;
	int sockfd;
	char buf[BUFFER_SIZE];
	heap_timer* timer;
};

class heap_timer
{
public:
	heap_timer() = default;
	heap_timer(int delay) : expire(time(nullptr) + delay) {};
	time_t expire;
	void (*cb_func)(client_data*);
	client_data* usr_data;
};

class time_heap
{
public:
	time_heap(int cap) throw(exception) :capacity(cap), cur_size(0)
	{
		arr = new heap_timer * [capacity];
		if (!arr)
			throw exception();
		for (auto i : arr)
			i = nullptr;
	}
	time_heap(heap_timer** init_arr, int size, int cap) throw(exception): cur_size(size), time_heap(cap)
	{
		if (capacity < size)
			throw exception();
		if (size != 0)
		{
			//初始化
			for (int i = 0; i < cur_size; ++i)
				arr[i] = init_arr[i];
			//从第一个非叶节点排序
			for (int i = (cur_size - 1) / 2 : i >= 0; --i)
				percolate(i)

		}

	}
	~time_heap()
	{
		for (auto i : arr)
			delete i;
		delete[] arr;
	}
	//尾部插入,进行上滤
	void add_timer(heap_timer* timer) throw(exception)
	{
		if (!timer) return;
		if (cur_size >= capacity)
			resize();
		int hole = cur_size++;
		int parent = 0;
		for (;hole>0;hole = parent)
		{
			parent = (hole-1) / 2;
			if(arr[parent]->expire<=timer->expire)
				break;
			arr[hole] = arr[parent];
		}
		arr[hole] = timer;
	}
	//删除
	void del_timer(heap_timer* timer)
	{
		if (!timer) return;
		/*仅仅将目标定时器的回调函数设置为空，即所谓的延迟销毁。这将节省真正删除该定时器造成的开销，但这样做容易使堆数组膨胀*/
		timer->cb_func=nullptr;
	}
	//获得顶部定时器
	heap_timer* top() const
	{
		if (empty()) return nullptr;
		return arr[0];
	}
	void pop()
	{
		if (empty()) return;
		if (arr[0])
		{
			delete arr[0];
			arr[0] = arr[--cur_size];
			percolate(0);
		}
	}

	void tick()
	{
		auto tmp = arr[0];
		time_t cur = time(nullptr);
		while (!empty())
		{
			if (!arr[0]) break;
			if (tmp->expire > cur)
				break;
			if(tmp->cb_func)
				tmp->cb_func(tmp->usr_data);
			pop();
			tmp = arr[0];
		}
	}

private:
	//下滤,确保堆数组中以第hole个节点作为根的子树拥有最小堆性质
	void percolate(int hole)
	{
		auto tmp = arr[hole];
		int child = hole * 2 + 1;
		while (child <= cur_size - 1)
		{
			if (arr[child + 1]->expire < arr[child]->expire)
				++child;
			if (arr[child]->expire < arr[hole]->expire)
				arr[hole] = arr[child];
			else
				break;
			hole = child;
			child = hole * 2 + 1;
		}
		arr[hole] = tmp;
	}

	void resize()throw(exception)
	{
		auto tmp = new heap_timer * [2 * capacity];
		if (!tmp)
			throw exception();
		for (auto i : tmp)
			i = nullptr;
		capacity = 2 * capacity; 
		for (int i = 0; i<cur_size; ++i) 
			tmp[i] = arr[i];  
		delete[] arr; 
		arr = tmp;
	
	}

	bool empty()const { return cur_size == 0; }

	heap_timer** arr;
	int capacity;
	int cur_size;
};


#endif // _TIME_MIN_HEAP_H__
