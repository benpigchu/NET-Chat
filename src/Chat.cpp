#include <vector>
#include <string>
#include <unordered_map>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "Chat.h"
#include "EventLoop.h"
#include "EventSource.h"
#include "TcpServer.h"
#include "TcpSocket.h"

#include <iostream>

struct UserData{
	::std::string name;
	::std::string password;
};

struct Session{
	::std::string inputBuffer;
	int userId=0;//0=not login
	int chattingUserId=0;//0=not chatting
};

void chat(){
	::std::vector<UserData> users={{"benpigchu","bpctest"},{"bpc2","bpctest2"},{"neko","nyanyan"},{"nyanko","nekoneko"}};//userId=index+1
	::std::unordered_map<TcpSocket*,Session> socketSessions;
	EventLoop loop;
	::std::unique_ptr<TcpServer> server=::std::make_unique<TcpServer>(::std::string("0.0.0.0"),7647);
	server->setOnConnectHandler([&socketSessions,&loop](TcpSocket* socket){
		socketSessions[socket];
		socket->setOnDataHandler([&socketSessions,&loop,socket](::std::string data){
			socketSessions[socket].inputBuffer+=data;
			while(socketSessions[socket].inputBuffer.length()>2){
				uint16_t l=*(uint16_t*)(socketSessions[socket].inputBuffer.c_str());
				int length=ntohs(l);
				if(socketSessions[socket].inputBuffer.length()>=2+length){
					::std::string packet=socketSessions[socket].inputBuffer.substr(2,2+length);
					socketSessions[socket].inputBuffer=socketSessions[socket].inputBuffer.substr(2+length);
					::std::cerr<<packet<<"\n";
				}else{
					break;
				}
			}
		});
		socket->setOnWritableHandler([&socketSessions,socket](){

		});
		socket->setOnConnectionLostHandler([&socketSessions,&loop,socket](){
			socketSessions.erase(socket);
			loop.deleteLater(socket);
		});
		loop<<socket;
	});
	loop<<server.get();
	loop.run();
}