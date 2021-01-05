#pragma once
#include <winsock2.h>
#pragma comment(lib,"ws2_32")       //����ws2_32.lib��

class CInitSock
{
public:
	CInitSock(BYTE minorVer = 2, BYTE major = 2)      //����Winsock��
	{
		WSADATA wsaData;
		WORD sockVersion = MAKEWORD(minorVer, major);
		if (::WSAStartup(sockVersion, &wsaData) != 0)
		{
			return;
		}
	}
	~CInitSock()
	{
		::WSACleanup(); //�ͷ�Winsock��
	}
};