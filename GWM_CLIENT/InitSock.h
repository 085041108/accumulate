#pragma once
#include <winsock2.h>
#pragma comment(lib,"ws2_32")       //¡¥Ω”ws2_32.libø‚

class CInitSock
{
public:
	CInitSock(BYTE minorVer = 2, BYTE major = 2)      //‘ÿ»ÎWinsockø‚
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
		::WSACleanup(); // Õ∑≈Winsockø‚
	}
};