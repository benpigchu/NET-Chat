#include <iostream>
#include <memory>
#include <string>
#include "EventLoop.h"
#include "EventSource.h"
#include "TcpServer.h"
#include "TcpSocket.h"
#include "Chat.h"

void echoTest(){
	EventLoop loop;
	::std::unique_ptr<TcpServer> server=::std::make_unique<TcpServer>(::std::string("0.0.0.0"),7647);
	server->setOnConnectHandler([&loop](TcpSocket* socket){
		socket->setOnDataHandler([&loop,socket](::std::string data){
			if(data.length()==0){
				loop.deleteLater(socket);
			}else{
				socket->attemptWrite(data);
			}
		});
		socket->setOnWritableHandler([](){});
		socket->setOnConnectionLostHandler([&loop,socket](){
				loop.deleteLater(socket);
		});
		loop<<socket;
	});
	loop<<server.get();
	loop.run();
}



int main(int argc,char** argv){
	::std::cout<<"building...\n";
	chat();
	return 0;
}