#include <function>
#include <memory>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include "TcpServer.h"

void TcpServer::setOnConnectHandler(::std::function<void(::std::unique_ptr<TcpSocket>)> handler){
	onConnectHandler=handler;
}

TcpServer::TcpServer(::std::string address,short port){
	fd=socket(AF_INET,SOCK_STREAM,0);
	if(fd<0){
		throw new ::std::runtime_error(strerror(errno));
	}
	addr.sin_family=AF_INET
	addr.sin_addr.s_addr=inet_addr(address.c_str());
	addr.sin_port=htons(port);
	if(bind(fd,(sockaddr*)addr,sizeof(sockaddr_in))<0){
		throw new ::std::runtime_error(strerror(errno));
	}
	if(listen(fd,64)<0){
		throw new ::std::runtime_error(strerror(errno));
	}
	eventType=EPOLLIN;
}

TcpServer::~TcpServer(){
	if(close(fd)<0){
		throw new ::std::runtime_error(strerror(errno));
	}
}

void TcpServer::handleEvent(uint32_t eventType){
	int newSocketFd=accept4(fd,(sockaddr*)addr,sizeof(sockaddr_in),SOCK_NONBLOCK);
	onConnectHandler(::std::unique_ptr<TcpSocket>(new TcpSocket(newSocketFd)));
}