#ifndef TCPSOCKET_H
#define TCPSOCKET_H
#include <function>
#include "EventSource.h"
class TcpServer;
class TcpSocket:public EventSource{
	friend class TcpServer;
	TcpSocket(int fd);
protected:
	void handleEvent(uint32_t eventType) override;
public:
	~TcpSocket();
};
#endif //TCPSOCKET_H