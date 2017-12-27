#include <stdexcept>
#include <unordered_set>
#include <sys/epoll.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include "EventLoop.h"
#include "EventSource.h"

#include <iostream>

EventLoop::EventLoop(){
	epoll_fd=epoll_create1(0);
	if(epoll_fd<0){
		throw ::std::runtime_error(strerror(errno));
	}
}

EventLoop::~EventLoop(){
	for(EventSource* source:sourceSet){
		source->detachFromEventLoop();
	}
	if(close(epoll_fd)<0){
		throw ::std::runtime_error(strerror(errno));
	}
}


void EventLoop::deleteLater(EventSource* source){
	pendingDeleteSet.insert(source);
}

void EventLoop::run(){
	running=true;
	const size_t max_events=64;
	epoll_event events[max_events];
	while(running){
		int eventCount=epoll_wait(epoll_fd,events,max_events,-1);
		if(eventCount<0){
			if(errno!=EINTR){
				throw ::std::runtime_error(strerror(errno));
			}
			continue;
		}
		for(int i=0;i<eventCount;i++){
			EventSource* source=reinterpret_cast<EventSource*>(events[i].data.ptr);
			source->handleEvent(events[i].events);
		}
		for(EventSource* deadPtr:pendingDeleteSet){
			delete deadPtr;
		}
		pendingDeleteSet.clear();
	}
}

void EventLoop::stop(){
	running=false;
}

EventLoop& EventLoop::operator<<(EventSource* source){
	sourceSet.insert(source);
	epoll_event ev;
	ev.events=source->eventType;
	ev.data.u64=0;
	ev.data.ptr=(void*)source;
	source->loop=this;
	if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,source->fd,&ev)<0){
		throw ::std::runtime_error(strerror(errno));
	}
	return *this;
}

EventLoop& EventLoop::operator>>(EventSource* source){
	sourceSet.erase(source);
	if(epoll_ctl(epoll_fd,EPOLL_CTL_DEL,source->fd,0)<0){
		throw ::std::runtime_error(strerror(errno));
	}
	return *this;
}