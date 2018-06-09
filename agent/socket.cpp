#include <stdio.h>
#include "socket.h"
#include "debug.h"

void Socket::init(unsigned long server, unsigned short port)
{
	_server = server;
	_port = port;
	
	WORD w_version_requested = MAKEWORD(1,1);
	WSADATA ws_adata;
	if( 0 != WSAStartup(w_version_requested, &ws_adata))
	{
		return ;
	}
	
	if((LOBYTE(ws_adata.wVersion) !=1) || (HIBYTE(ws_adata.wVersion) != 1))
	{
		WSACleanup();
		return;
	}
	
	sock = socket(AF_INET,SOCK_STREAM,0);
	
	addr_srv.sin_addr.s_addr = _server;
	addr_srv.sin_family =AF_INET;
	addr_srv.sin_port = htnos(_port);
}

void Socket::connect()
{
	int count = 0;
	while(0 != ::connect(sock,(sockaddr*)&addr_srv,sizeof(sockaddr)) && count < 10)
	{
		count++;
		Sleep(10);
	};
}

static char recv_data_buf[100000] = {0};
static unsigned long long recv_time = 0;
string Socket::recv()
{
	char size_buff[6] = {'\0'};
	int ret = ::recv(sock,size_buff, sizeof(size_buff) - 1,0);
	if(ret < 0)
	{
		connect();
		ret = ::recv(sock,size_buff,sizeof(size_buff) -1,0);
	}
	if(ret < 0)
	{
		return string("");
	}
	
	int length = atoi(size_buff);
	assert(length < 99999);
	recv_time = GetMicroSec();
	recv_data_buf[length] = 0;
	int length_count = 0;
	char * buf_start = recv_data_buf;
	do{
		ret = ::recv(sock,buf_start,length - length_count,0);
		if(ret < 0)
		{
			connect();
			ret = :: recv(sock,buf_start,length - length_count,0);
		}
		else
		{
			length_count += ret;
			buf_start = recv_data_buf + length_count;
		}
	}while(ret >= 0 && length_count < length);
	
	if(ret > 0)
	{
		return string(recv_data_buf);
	}
	else
		return string("");
}

int Socket::send(string buf)
{
	if(buf.length() > 99999)
		return -1;
	char len_buf[6] = {'\0'};
	
	sprintf(len_buf,sizeof(len_buf),"%05d",buf.length());
	int ret = ::send(sock,len_buf,5,0);
	
	if(ret < 0)
	{
		connect();
		ret = ::send(sock,len_buf,5,0);
	}
	ret = ::send(sock,buf.c_str(),buf.length(),0);
	if(ret < 0)
	{
		connect();
		ret = ::send(sock,buf.c_str(),buf.length(),0);
	}
	
	unsigned long long delta = GetMicroSec() - recv_time;
	if(delta > _longest_time)
		_longest_time = delta;

	return ret;
}

Socket::~Socket()
{
	shutdonw(sock,SD_BOTH);
	closesocket(sock);
	WSACleanup();
}
