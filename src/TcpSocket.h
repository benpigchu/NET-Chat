#ifndef TCPSOCKET_H
#define TCPSOCKET_H
#include <functional>
#include <string>
#include "EventSource.h"
class TcpServer;
class TcpSocket:public EventSource{
	friend class TcpServer;
	TcpSocket(int sofd);
	::std::function<void(::std::string)> onDataHandler;
	::std::function<void()> onWritableHandler;
	::std::function<void()> onConnectionLostHandler;
	::std::string buffer;// size>0 <=> wait for next event
	bool connectionLost=false;
	void lostConnection();
protected:
	void handleEvent(uint32_t eventType)override;
public:
	void setOnDataHandler(::std::function<void(::std::string)> handler);//a zero length buffer is eof sign
	void setOnWritableHandler(::std::function<void()> handler);
	void setOnConnectionLostHandler(::std::function<void()> handler);
	bool attemptWrite(::std::string content);//return: should i wait for next writable event. If you do not wait, buffer may bang
	~TcpSocket();
};
#endif //TCPSOCKET_H