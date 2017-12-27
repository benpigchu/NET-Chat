#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <string.h>
#include <errno.h>
#include "TcpSocket.h"

#include <iostream>

void TcpSocket::setOnDataHandler(::std::function<void(::std::string)> handler){
	onDataHandler=handler;
}

void TcpSocket::setOnWritableHandler(::std::function<void()> handler){
	onWritableHandler=handler;
}

void TcpSocket::setOnConnectionLostHandler(::std::function<void()> handler){
	onConnectionLostHandler=handler;
}

TcpSocket::TcpSocket(int sofd){
	fd=sofd;
	eventType=EPOLLIN|EPOLLOUT|EPOLLRDHUP|EPOLLET;
}

TcpSocket::~TcpSocket(){}

bool TcpSocket::attemptWrite(::std::string content){
	if(buffer.length()==0){
		int length=send(fd,content.c_str(),content.length(),0);
		if(length<content.length()){
			if(length<0){
				if((errno!=EAGAIN)&&(errno!=EWOULDBLOCK)){
					throw ::std::runtime_error(strerror(errno));
				}
			}
			int nextPos=length<0?0:length;
			buffer=content.substr(nextPos);
			return false;
		}
		return true;
	}else{
		buffer+=content;
		return false;
	}
}

void TcpSocket::lostConnection(){
	if(!connectionLost){
		onConnectionLostHandler();
	}
	connectionLost=true;
}

void TcpSocket::handleEvent(uint32_t eventType){
	if((eventType&EPOLLIN)!=0){
		while(true){
			const size_t bufferLength=4096;
			char buffer[bufferLength];
			int length=recv(fd,buffer,bufferLength,0);
			if(length<0){
				if((errno!=EAGAIN)&&(errno!=EWOULDBLOCK)){
					if(errno==ECONNREFUSED){
						lostConnection();
					}else{
						throw ::std::runtime_error(strerror(errno));
					}
				}
				break;
			}
			onDataHandler(::std::string(buffer,length));
			if(length==0){
				break;
			}
		}
	}
	if((eventType&EPOLLERR)!=0){
		lostConnection();
	}
	if((eventType&EPOLLRDHUP)!=0){
		onDataHandler("");
	}
	if((eventType&EPOLLOUT)!=0){
		while(buffer.length()>0){
			int length=send(fd,buffer.c_str(),buffer.length(),0);//my buffer should not be too long
			if(length<0){
				if((errno!=EAGAIN)&&(errno!=EWOULDBLOCK)){
					if(errno==ECONNREFUSED){
						lostConnection();
					}else{
						throw ::std::runtime_error(strerror(errno));
					}
				}
				break;
			}
			int nextPos=length<0?0:length;
			buffer=buffer.substr(nextPos);
		}
		if(buffer.length()==0){
			onWritableHandler();
		}
	}
}