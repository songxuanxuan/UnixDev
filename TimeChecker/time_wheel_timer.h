/*ʱ��������洢��ʱ��*/
#ifndef _TIME_WHEEL_TIMER_H
#define _TIME_WHEEL_TIMER_H

#include <ctime>
#include <netinet/in.h>
#include <cstdio>
#define BUFFER_SIZE 64
class tw_timer;
struct client_data
{
	sockaddr_in addr;
	int sockfd;
	char buf[BUFFER_SIZE];
	tw_timer* timer;
};


//��ʱ��
class tw_timer
{
public:
	tw_timer(int rot, int ts) :next(nullptr), prev(nullptr), rotation(rot), time_slot(ts) {};

	int rotation;	//����Ȧ
	int time_slot;	//���ĸ�slot
	void (*cb_func)(client_data*);
	client_data* usr_data;
	tw_timer* prev;
	tw_timer* next;

};

class time_wheel
{
public:
	time_wheel() :cur_slot(0)
	{
		for (auto i : slot)
			i = nullptr;

	};
	~time_wheel()
	{
		for (auto i : slot)
		{
			auto tmp = i;
			while (tmp)
			{
				i = i->next;
				delete tmp;
				tmp = i;
			}
		}
	};
	//���ݳ�ʱʱ�䴴����ʱ������,���ض�ʱ��
	tw_timer* add_timer(int timeout)
	{
		if (timeout < 0)	return nullptr;
		int ticks = timeout < SI ? 1 : timeout / SI;
		int rotation = ticks / N;
		int ts = (cur_slot + (ticks % N)) % N;
		auto timer = new tw_timer(rotation, ts);
		if (!slots[ts])
		{
			printf("add timer,rotation is%d,ts is%d,cur_slot is%d\n", rotation, ts, cur_slot);
			slots[ts] = timer;
		}
		else
		{
			timer->next = slots[ts];
			slots[ts]->prev = timer;
			slots[ts] = timer;
		}
		return timer;
	}

	//ɾ��,������һ��
	tw_timer* del_timer(tw_timer* timer)
	{
		if (!timer)	return;
		int ts = timer->time_slot;
		if (timer == slots[ts])
		{
			slots[ts] = slots[ts]->next;
			if (slots[ts])
				slots[ts]->prev = nullptr;
			delete timer;
			return slots[ts];
		}
		else
		{
			auto next = timer->next;
			timer->prev->next = next;
			if (next)
				next->prev = timer->prev;
			delete timer;
			return next;
		}
	}

	//ʱ����������
	void tick()
	{
		auto tmp = slots[cur_slot];
		printf("current slot is%d\n", cur_slot);
		while (tmp)
		{
			if (tmp->rotation > 0)
			{
				--tmp->rotation;
				tmp = tmp->next;
			}
			else
				tmp = del_timer(tmp);
		}
		cur_slot = (++cur_slot) % N;
	}

private:
	//slot ����
	static const int N = 60;
	//time slot interval
	static const int SI = 1;
	tw_timer* slots[N];
	int cur_slot;
};


#endif // _TIME_WHEEL_TIMER_H__
