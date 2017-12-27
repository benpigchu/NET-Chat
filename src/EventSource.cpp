#include <stdexcept>
#include <string.h>
#include <errno.h>
#include <cstddef>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "EventSource.h"
#include "EventLoop.h"

void EventSource::detachFromEventLoop(){
	loop=nullptr;
}

EventSource::~EventSource(){
	if(loop!=nullptr){
		(*loop)>>this;
	}
	if(fd>=0){
		if(close(fd)<0){
			throw new ::std::runtime_error(strerror(errno));
		}
	}
}