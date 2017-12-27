#ifndef TCPSERVER_H
#define TCPSERVER_H
#include <functional>
#include <memory>
#include <string>
#include <netinet/in.h>
#include "EventSource.h"
#include "TcpSocket.h"
class TcpServer:public EventSource{//tcp server. to be simple, only over ipv4
	sockaddr_in addr;
protected:
	void handleEvent(uint32_t eventType)override;
	::std::function<void(TcpSocket*)> onConnectHandler;
public:
	void setOnConnectHandler(::std::function<void(TcpSocket*)> handler);
	TcpServer(::std::string address,short port);
	~TcpServer();
};
#endif //TCPSERVER_H