#ifndef EVENTLOOP_H
#define EVENTLOOP_H
#include <unordered_set>
#include "EventSource.h"

class EventLoop{//simple wrapper of epoll interface
	int epoll_fd=-1;
	bool running=false;
	::std::unordered_set<EventSource*> sourceSet;
	::std::unordered_set<EventSource*> pendingDeleteSet;
public:
	EventLoop();
	void run();
	void stop();
	void deleteLater(EventSource* source);
	EventLoop& operator<<(EventSource* source);
	EventLoop& operator>>(EventSource* source);
	~EventLoop()noexcept;
};

#endif //EVENTLOOP_H