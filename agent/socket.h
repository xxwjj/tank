#pragma once
#include <WinSock2.h>
#include <string>

using namespace std;

#define CONNECTOR (Socket::getInstance())

class Socket
{
	public:
	Socket()
	{
	};
	~Socket();
	void init(unsigned long server,unsigned short port);
	void connect();
	string recv();
	int send(string buf);
	static Socket*getInstance()
	{
		static Socket instance;
		return &instance;
	}
	unsigned long long _longest_time;
private:
   unsigned long _server;
   unsigned short _port;
   SOCKET sock;
   sockaddr_in addr_srv;
 };
