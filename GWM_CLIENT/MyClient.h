#pragma once
#include <iostream>
#include<WS2tcpip.h>
#include <windows.h>
#include <time.h>
#include "MyLock.h"
#include "InitSock.h"
#include "MyString.h"
#include "./log4cpp/Clog4Util.h"
using namespace std;

#define API_ERROR 0
#define API_SUCC  1
#define REC_LEN	  1024
#define LOG_LEN	  1024

typedef struct DATASTRUCT 
{
	float f_press;
	float f_temperature;
	int n_standard;
	float f_standard;
	int n_work;
	float f_immediate;
	int   n_update_time;
	DATASTRUCT() {
		f_press = 0;
		f_temperature = 0;
		n_standard = 0;
		f_standard = 0;
		n_work = 0;
		f_immediate = 0;
		n_update_time = 0;
	}
}DataStruct;

class MyClient {
public:
	MyClient(int port, string str_ip, Mutex* pmut_lock);
	~MyClient();

public:
	friend void MyCreateThread(LPVOID param);
	int StartMyThread();
	int GetValue(DataStruct& data_struct);
	int ReStartMyThread();
private:
	int Connect();
	int RecvData();
	int CloseSocket();
	int Init();
	int StartRun();
private:
	int Analyse(string str_rec);
	string HexToStr(char* p_hex, int len);
private:
	sockaddr_in m_sock_server;
	SOCKET m_sock_client;
	int m_port;
	string m_ip;
	unsigned long m_thread_id;
	Mutex* m_pmut_lock;
	char m_log_msg[LOG_LEN];
private:
	float m_f_press;
	float m_f_temperature;
	int m_n_standard;
	float m_f_standard;
	int m_n_work;
	float m_f_immediate;
	int   m_update_time;
	string m_str_rec;
	bool  m_get_flag;
	HANDLE m_h_thread;
public:
	int   m_type;
	bool  m_con_flag;

};
