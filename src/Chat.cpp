#include <vector>
#include <string>
#include <deque>
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
	::std::deque<::std::string> outputBuffer;
	int userId=0;//0=not login
	int chattingUserId=0;//0=not chatting
	bool writable=true;
};

void chat(){
	::std::vector<UserData> users={{"benpigchu","bpctest"},{"bpc2","bpctest2"},{"neko","nyanyan"},{"nyanko","nekoneko"}};//userId=index+1
	::std::unordered_map<TcpSocket*,Session> socketSessions;
	EventLoop loop;
	::std::unique_ptr<TcpServer> server=::std::make_unique<TcpServer>(::std::string("0.0.0.0"),7647);
	server->setOnConnectHandler([&socketSessions,&loop](TcpSocket* socket){
		socketSessions[socket];
		auto sendPacket=[&socketSessions,socket](::std::string data){
			uint16_t l=data.length();
			uint16_t lt=htons(l);
			::std::string packet=::std::string((char*)(&lt),2)+data;
			::std::cerr<<(uint16_t)(packet[0])<<" "<<(uint16_t)(packet[1])<<"\n";
			if(socketSessions[socket].writable){
				socketSessions[socket].writable=socket->attemptWrite(packet);
			}else{
				socketSessions[socket].outputBuffer.push_back(packet);
			}
		};
		socket->setOnDataHandler([&socketSessions,&loop,socket,&sendPacket](::std::string data){
			socketSessions[socket].inputBuffer+=data;
			while(socketSessions[socket].inputBuffer.length()>2){
				uint16_t l=*(uint16_t*)(socketSessions[socket].inputBuffer.c_str());
				int length=ntohs(l);
				if(socketSessions[socket].inputBuffer.length()>=2+length){
					::std::string packet=socketSessions[socket].inputBuffer.substr(2,2+length);
					socketSessions[socket].inputBuffer=socketSessions[socket].inputBuffer.substr(2+length);
					{
						sendPacket(::std::string("iz"));
					}
				}else{
					break;
				}
			}
		});
		socket->setOnWritableHandler([&socketSessions,socket](){
			socketSessions[socket].writable=true;
			while((!socketSessions[socket].outputBuffer.empty())&&socketSessions[socket].writable){
				::std::string packet=socketSessions[socket].outputBuffer.front();
				socketSessions[socket].outputBuffer.pop_front();
				socketSessions[socket].writable=socket->attemptWrite(packet);
			}
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