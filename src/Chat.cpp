#include <vector>
#include <string>
#include <deque>
#include <unordered_map>
#include <unordered_set>
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
	::std::vector<::std::unordered_set<int>> friendList(users.size());
	::std::vector<::std::deque<::std::string>> messageCache(users.size());
	EventLoop loop;
	::std::unique_ptr<TcpServer> server=::std::make_unique<TcpServer>(::std::string("0.0.0.0"),7647);
	auto sendPacket=[&socketSessions](TcpSocket* socket,::std::string data){
		uint16_t l=data.length();
		uint16_t lt=htons(l);
		::std::string packet=::std::string((char*)(&lt),2)+data;
	::std::cerr<<"--------------1\n";
	::std::cerr<<(socket)<<"--------------1\n";
	::std::cerr<<(&socketSessions)<<"--------------1\n";
		if(socketSessions[socket].writable){
	::std::cerr<<"--------------l1\n";
			socketSessions[socket].writable=socket->attemptWrite(packet);
	::std::cerr<<"--------------l2\n";
		}else{
	::std::cerr<<"--------------r1\n";
			socketSessions[socket].outputBuffer.push_back(packet);
	::std::cerr<<"--------------r2\n";
		}
	};
	auto transmitPackage=[&socketSessions,&sendPacket,&users,&messageCache](int userId,::std::string data){
		for(auto pair:socketSessions){
			if(pair.second.userId==userId+1){
				sendPacket(pair.first,data);
				return;
			}
		}
		messageCache[userId].push_back(data);
	};
	server->setOnConnectHandler([&socketSessions,&loop,&users,&friendList,&messageCache,&sendPacket,&transmitPackage](TcpSocket* socket){
		socketSessions[socket];
		socket->setOnDataHandler([&socketSessions,&loop,socket,&sendPacket,&users,&friendList,&messageCache,&transmitPackage](::std::string data){
			if(data.length()==0){
		::std::cerr<<(&socketSessions)<<"--------------xx\n";
				socketSessions.erase(socket);
				loop.deleteLater(socket);
		::std::cerr<<(&socketSessions)<<"--------------xx\n";
				::std::cerr<<"--------------xx\n";
				return;
			}
				::std::cerr<<"--------------xx?\n";
			socketSessions[socket].inputBuffer+=data;
			while(socketSessions[socket].inputBuffer.length()>2){
				uint16_t l=*(uint16_t*)(socketSessions[socket].inputBuffer.c_str());
				int length=ntohs(l);
				if(socketSessions[socket].inputBuffer.length()>=2+length){
					::std::string packet=socketSessions[socket].inputBuffer.substr(2,2+length);
					socketSessions[socket].inputBuffer=socketSessions[socket].inputBuffer.substr(2+length);
					if(packet.length()>0){
						if(packet[0]==0){//login
							if(packet.length()>=33){
								::std::string uRaw=packet.substr(1,17)+::std::string("\0",1);
								::std::string pRaw=packet.substr(17,33)+::std::string("\0",1);
								::std::string username=::std::string(uRaw.c_str());
								::std::string password=::std::string(pRaw.c_str());
								int i=0;
								for(;i<users.size();i++){
									if(users[i].name==username){
										if(users[i].password==password){
											for(auto pair:socketSessions){
												if(pair.second.userId==i+1){
													sendPacket(socket,::std::string("\0\3",2));
													goto error;
												}
											}
											socketSessions[socket].userId=i+1;
											sendPacket(socket,::std::string("\0\0",2));
											while(!(messageCache[i].empty())){
												::std::cerr<<messageCache[i].size()<<"\n";
												sendPacket(socket,messageCache[i].front());
												messageCache[i].pop_front();
											}
										}else{
											sendPacket(socket,::std::string("\0\x02",2));
										}
										error:break;
									}
								}
								if(i==users.size()){
									sendPacket(socket,::std::string("\0\x01",2));
								}
							}
						}else if(packet[0]==1){//search
							if(socketSessions[socket].userId==0){
								sendPacket(socket,"\xff");
								return;
							}
							::std::string data="\x01";
							for(UserData user:users){
								::std::string name=user.name;
								name.resize(16,'\0');
								data+=name;
							}
							sendPacket(socket,data);
						}else if(packet[0]==2){//profile
							if(socketSessions[socket].userId==0){
								sendPacket(socket,"\xff");
								return;
							}
							::std::string data="\x02";
							::std::string name=socketSessions[socket].userId==0?::std::string():users[socketSessions[socket].userId-1].name;
							::std::string chattingName=socketSessions[socket].chattingUserId==0?::std::string():users[socketSessions[socket].chattingUserId-1].name;
							name.resize(16,'\0');
							data+=name;
							chattingName.resize(16,'\0');
							data+=chattingName;
							sendPacket(socket,data);
						}else if(packet[0]==3){//add
							if(socketSessions[socket].userId==0){
								sendPacket(socket,"\xff");
								return;
							}
							if(packet.length()>=17){
								::std::string uRaw=packet.substr(1,17)+::std::string("\0",1);
								::std::string username=::std::string(uRaw.c_str());
								if(username==users[socketSessions[socket].userId-1].name){
									sendPacket(socket,::std::string("\x03\x02",2));
									return;
								}
								int i=0;
								for(;i<users.size();i++){
									if(users[i].name==username){
										auto& friends=friendList[socketSessions[socket].userId-1];
										if(friends.find(i)!=friends.end()){
											sendPacket(socket,::std::string("\x03\x03",2));
										}else{
											friends.insert(i);
											sendPacket(socket,::std::string("\x03\0",2));
										}
										break;
									}
								}
								if(i==users.size()){
									sendPacket(socket,::std::string("\03\x01",2));
								}
							}
						}else if(packet[0]==4){//ls
							if(socketSessions[socket].userId==0){
								sendPacket(socket,"\xff");
								return;
							}
							::std::string data="\x04";
							for(int user:friendList[socketSessions[socket].userId-1]){
								::std::string name=users[user].name;
								name.resize(16,'\0');
								data+=name;
							}
							sendPacket(socket,data);
						}else if(packet[0]==5){//chat/exit
							if(socketSessions[socket].userId==0){
								sendPacket(socket,"\xff");
								return;
							}
							if(packet.length()>=17){
								::std::string uRaw=packet.substr(1,17)+::std::string("\0",1);
								::std::string username=::std::string(uRaw.c_str());
								if(username==""){
									socketSessions[socket].chattingUserId=0;
									sendPacket(socket,::std::string("\x05\0",2));
									return;
								}
								if(username==users[socketSessions[socket].userId-1].name){
									sendPacket(socket,::std::string("\x05\x02",2));
									return;
								}
								int i=0;
								for(;i<users.size();i++){
									if(users[i].name==username){
										socketSessions[socket].chattingUserId=i+1;
										sendPacket(socket,::std::string("\x05\0",2));
										break;
									}
								}
								if(i==users.size()){
									sendPacket(socket,::std::string("\05\x01",2));
								}
							}
						}else if(packet[0]==6){//smsg
							if(socketSessions[socket].userId==0){
								sendPacket(socket,"\xff");
								return;
							}
							if(socketSessions[socket].chattingUserId<=0){
								sendPacket(socket,"\xfe");
							}
							::std::string name=users[socketSessions[socket].userId-1].name;
							name.resize(16,'\0');
							transmitPackage(socketSessions[socket].chattingUserId-1,"\x06"+name+packet.substr(1));
						}else if(packet[0]==7){//smsg
						::std::cerr<<packet.length()<<"\n";
							if(socketSessions[socket].userId==0){
								sendPacket(socket,"\xff");
								return;
							}
							if(socketSessions[socket].chattingUserId<=0){
								sendPacket(socket,"\xfe");
							}
							::std::string name=users[socketSessions[socket].userId-1].name;
							name.resize(16,'\0');
							transmitPackage(socketSessions[socket].chattingUserId-1,"\x07"+name+packet.substr(1));
						}
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