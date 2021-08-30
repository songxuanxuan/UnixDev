#ifndef _TIME_UTIL_H
#define _TIME_UTIL_H
#include <ctime>
#include <netinet/in.h>

#define BUFFER_SIZE 64

class util_timer;
struct client_data
{
	sockaddr_in address;
	int sockfd;
	char buf[BUFFER_SIZE];
	util_timer* timer;
};

class util_timer
{
public:
	util_timer() :prev(nullptr), next(nullptr) {};

	time_t expire;	//任务超时绝对时间
	void (*cb_func)(client_data*);
	client_data* usr_data;

	util_timer* prev;
	util_timer* next;
};

class sort_time_lst
{
public:
	sort_time_lst() :head(nullptr), tail(nullptr) {};
	~sort_time_lst()
	{
		util_timer* tmp = head;
		while (tmp)
		{
			head = tmp->next;
			delete tmp;
			tmp = head;
		}
	}
	void add_time(util_timer* timer)
	{
		if (!timer)	return;
		if (!head)
		{
			head = tail = timer;
			return;
		}
		if (timer->expire <= head->expire)
		{
			timer->next = head;
			head->prev = timer;
			head = timer;
			return;
		}
		add_time(timer, head);
	}
	//只考虑向后调整
	void adjust_timer(util_timer* timer)
	{
		if (!timer)	return;
		util_timer* tmp = timer->next;
		if (!tmp || (timer->expire <= tmp->expire))	return;
		if (timer == head)
		{
			head = head->next;
			head->prev = nullptr;
			timer->next = nullptr;
			add_time(timer, head);
		}
		else
		{
			timer->prev->next = tmp;
			tmp->prev = timer->prev;
			timer->next = nullptr;
			timer->prev = nullptr;
			add_time(timer, tmp);
		}
	}

	void del_timer(util_timer* timer)
	{
		if (!timer)	return;
		if (timer == head && timer == tail)
		{
			delete timer;
			head = tail = nullptr;
			return;
		}
		//下面都是两个或以上的情况
		if (timer == head)
		{
			head = head->next;
			head->prev = nullptr;
			delete timer;
			return;
		}
		if (timer == tail)
		{
			tail = tail->prev;
			tail->next = nullptr;
			delete timer;
			return;
		}
		timer->prev->next = timer->next;
		timer->next->prev = timer->prev;
		delete timer;
	}
	void tick()
	{
		if (!head) return;
		util_timer* tmp = head;
		time_t now = time(nullptr);
		while (tmp)
		{
			if (tmp->expire > now) break;
			tmp->cb_func(tmp->usr_data);
			del_timer(tmp);
			tmp = head;
		}
	}
private:

	//从head后一个节点开始寻找插入
	void add_time(util_timer* timer, util_timer* head)
	{
		util_timer* prev = head;
		util_timer* tmp = prev->next;
		while (tmp)
		{
			if (tmp->expire >= timer->expire)
			{
				prev->next = timer;
				timer->prev = prev;
				tmp->prev = timer;
				timer->next = tmp;
				break;
			}
			prev = tmp;
			tmp = tmp->next;
		}
		if (!tmp)
		{
			timer->next = nullptr;
			timer->prev = tail;
			tail->next = timer;
			tail = timer;
		}
	}
	util_timer* head;
	util_timer* tail;
};



#endif // _TIME_UTIL_H__
