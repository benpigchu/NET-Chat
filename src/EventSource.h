#ifndef EVENTSOURCE_H
#define EVENTSOURCE_H
#include <cstddef>

class EventLoop;
class EventSource{
	friend class EventLoop;
protected:
	EventLoop* loop=nullptr;
	int fd=-1;
	uint32_t eventType=0;
	virtual void handleEvent(uint32_t eventType)=0;
	virtual void detachFromEventLoop();
public:
	~EventSource();
};

#endif //EVENTSOURCE_H