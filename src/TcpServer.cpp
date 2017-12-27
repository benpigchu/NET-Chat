#include <functional>
#include <memory>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <string.h>
#include <errno.h>
#include "TcpServer.h"
#include "TcpSocket.h"

#include <iostream>

void TcpServer::setOnConnectHandler(::std::function<void(TcpSocket*)> handler){
	onConnectHandler=handler;
}

TcpServer::TcpServer(::std::string address,short port){
	fd=socket(AF_INET,SOCK_STREAM,0);
	if(fd<0){
		throw ::std::runtime_error(strerror(errno));
	}
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=inet_addr(address.c_str());
	addr.sin_port=htons(port);
	if(bind(fd,(sockaddr*)(&addr),sizeof(sockaddr_in))<0){
		throw ::std::runtime_error(strerror(errno));
	}
	if(listen(fd,64)<0){
		throw ::std::runtime_error(strerror(errno));
	}
	eventType=EPOLLIN;
}

TcpServer::~TcpServer(){}

void TcpServer::handleEvent(uint32_t eventType){
	socklen_t sin_len = sizeof(sockaddr_in);
	int newSocketFd=accept4(fd,(sockaddr*)(&addr),&sin_len,SOCK_NONBLOCK);
	if(newSocketFd<0){
		throw ::std::runtime_error(strerror(errno));
	}
	onConnectHandler(new TcpSocket(newSocketFd));
}