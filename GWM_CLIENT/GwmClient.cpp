#include <iostream>
#include<WS2tcpip.h>
#include <vector>

#include "InitSock.h"
#include "MyClient.h"
#include "MyLock.h"
#include "./kafka/SendKafka.h"

using namespace std;

typedef struct EQUCONFIG
{
	string str_ip;
	int port;
	string str_press;
	string str_temperature;
	string str_standard;
	string str_work;
	string str_immediate;
	int n_type;
	EQUCONFIG()
	{
		str_ip = "";
		port = 0;
		str_press = "";
		str_temperature = "";
		str_standard = "";
		str_work = "";
		str_immediate = "";
		n_type = 0;
	}
}EquConf;

typedef struct STRCLIENT
{
	MyClient* my_client;
	DataStruct data_struct;
	EquConf  equ_conf;
	STRCLIENT()
	{
		my_client = NULL;
	}
}StrClient;

void InputConfig(string str_name, EquConf& equ_conf)
{
	int n_addr = str_name.find(",");
	string str_temp;
	if (-1 != n_addr)
	{
		str_temp = str_name.substr(0, n_addr);
		int n_temp = str_temp.find(":");
		if (-1 == n_temp)
		{
			char ch_log[32] = { 0 };
			sprintf_s(ch_log, "Analy name fail,please check!!!");
			log4Util::Info(ch_log);
		}
		if (-1 != str_temp.find("TX"))
		{
			equ_conf.n_type = 1;
		}
		str_temp = str_temp.substr(n_temp+1);
		n_temp = str_temp.find(":");
		if (-1 == n_temp)
		{
			char ch_log[32] = { 0 };
			sprintf_s(ch_log, "Analy name fail,please check!!!");
			log4Util::Info(ch_log);
		}
		equ_conf.str_ip = str_temp.substr(0, n_temp);
		equ_conf.port = atoi(str_temp.substr(n_temp + 1).c_str());
	}
	// 分别区分DAM设备和天信流量计
	if (1 == equ_conf.n_type)
	{
		str_name = str_name.substr(n_addr + 1);
		n_addr = str_name.find(",");
		if (-1 == n_addr)
		{
			char ch_log[32] = { 0 };
			sprintf_s(ch_log, "Analy name fail,please check!!!");
			log4Util::Info(ch_log);
		}
		equ_conf.str_immediate = str_name.substr(0, n_addr);

		str_name = str_name.substr(n_addr + 1);
		n_addr = str_name.find(",");
		if (-1 == n_addr)
		{
			char ch_log[32] = { 0 };
			sprintf_s(ch_log, "Analy name fail,please check!!!");
			log4Util::Info(ch_log);
		}
		equ_conf.str_standard = str_name.substr(0, n_addr);

		str_name = str_name.substr(n_addr + 1);
		n_addr = str_name.find(",");
		if (-1 == n_addr)
		{
			char ch_log[32] = { 0 };
			sprintf_s(ch_log, "Analy name fail,please check!!!");
			log4Util::Info(ch_log);
		}
		equ_conf.str_temperature = str_name.substr(0, n_addr);

		equ_conf.str_press = str_name.substr(n_addr + 1);
	}
	else
	{
		str_name = str_name.substr(n_addr + 1);
		n_addr = str_name.find(",");
		if (-1 == n_addr)
		{
			char ch_log[32] = { 0 };
			sprintf_s(ch_log, "Analy name fail,please check!!!");
			log4Util::Info(ch_log);
		}
		equ_conf.str_press = str_name.substr(0, n_addr);

		str_name = str_name.substr(n_addr + 1);
		n_addr = str_name.find(",");
		if (-1 == n_addr)
		{
			char ch_log[32] = { 0 };
			sprintf_s(ch_log, "Analy name fail,please check!!!");
			log4Util::Info(ch_log);
		}
		equ_conf.str_temperature = str_name.substr(0, n_addr);

		str_name = str_name.substr(n_addr + 1);
		n_addr = str_name.find(",");
		if (-1 == n_addr)
		{
			char ch_log[32] = { 0 };
			sprintf_s(ch_log, "Analy name fail,please check!!!");
			log4Util::Info(ch_log);
		}
		equ_conf.str_standard = str_name.substr(0, n_addr);

		str_name = str_name.substr(n_addr + 1);
		n_addr = str_name.find(",");
		if (-1 == n_addr)
		{
			char ch_log[32] = { 0 };
			sprintf_s(ch_log, "Analy name fail,please check!!!");
			log4Util::Info(ch_log);
		}
		equ_conf.str_work = str_name.substr(0, n_addr);

		equ_conf.str_immediate = str_name.substr(n_addr + 1);
	}
}

void ReadConfig(vector<EquConf>& vec_equ_conf)
{
	char   inBuf[1024];
	int coun = 1;
	while (true)
	{
		EquConf equ_conf;
		char ch_temp[32] = { 0 };
		sprintf_s(ch_temp, "name%d", coun++);
		int ret = GetPrivateProfileString("point",
			ch_temp,
			"Error:failed",
			inBuf,
			1024,
			"C:\\configure\\opc.ini");
		std::string str_name;
		str_name.assign(inBuf);
		if (-1 != str_name.find("Error:failed"))
		{
			break;
		}
		else
		{
			log4Util::Info(str_name.c_str());
			InputConfig(str_name, equ_conf);
			vec_equ_conf.push_back(equ_conf);
		}
	}
	
}

int main()
{
	//初始化套接字库
	CInitSock initSock;
	vector<StrClient> vec_client;
	MyClient* p_my_client;
	Mutex* p_mut_lock = new Mutex();
	MyProducer *m_pProducer;
	m_pProducer = MyProducer::getInstance();
	vector<EquConf> vec_equ_conf;

	// 初始化日志配置
	log4Util::Init("C:\\configure\\DAM\\log4cpp.conf");
	
	/*char strLog_[1024] = { 0 };
	sprintf_s(strLog_, "test error log,%s:%d.", __FILE__, __LINE__);
	log4Util::Error(strLog_);*/

	// 读取采集点配置
	ReadConfig(vec_equ_conf);
	vector<EquConf>::iterator it = vec_equ_conf.begin();
	for (; it != vec_equ_conf.end(); it++)
	{
		StrClient str_client;
		p_my_client = new MyClient(it->port, it->str_ip.c_str(), p_mut_lock);
		p_my_client->m_type = it->n_type;
		p_my_client->StartMyThread();
		str_client.my_client = p_my_client;

		str_client.equ_conf.str_ip = it->str_ip;
		str_client.equ_conf.port = it->port;
		str_client.equ_conf.n_type = it->n_type;
		str_client.equ_conf.str_press = it->str_press;
		str_client.equ_conf.str_temperature = it->str_temperature;
		str_client.equ_conf.str_standard = it->str_standard;
		str_client.equ_conf.str_work = it->str_work;
		str_client.equ_conf.str_immediate = it->str_immediate;

		vec_client.push_back(str_client);
	}
	while (true)
	{
		vector<StrClient>::iterator it = vec_client.begin();
		for (; it != vec_client.end(); it++)
		{
			DataStruct data_struct;
			it->my_client->GetValue(data_struct);
			if (it->data_struct.n_update_time < data_struct.n_update_time)
			{
				it->data_struct.n_update_time = data_struct.n_update_time;
				it->data_struct.f_press = data_struct.f_press;
				it->data_struct.f_temperature = data_struct.f_temperature;
				it->data_struct.n_standard = data_struct.n_standard;
				it->data_struct.f_standard = data_struct.f_standard;
				it->data_struct.n_work = data_struct.n_work;
				it->data_struct.f_immediate = data_struct.f_immediate;

				// sendto kafka
				char ch_value[64] = { 0 };
				string str_value;
				sprintf_s(ch_value, "%f", it->data_struct.f_press);
				str_value.assign(ch_value);
				m_pProducer->SendKafaData(str_value, it->equ_conf.str_press);

				memset(ch_value, 0, 64);
				sprintf_s(ch_value, "%f", it->data_struct.f_temperature);
				str_value.assign(ch_value);
				m_pProducer->SendKafaData(str_value, it->equ_conf.str_temperature);

				memset(ch_value, 0, 64);
				if (1 == it->equ_conf.n_type)
				{
					sprintf_s(ch_value, "%f", it->data_struct.f_standard);
				}
				else
				{
					sprintf_s(ch_value, "%d", it->data_struct.n_standard);
				}
				str_value.assign(ch_value);
				m_pProducer->SendKafaData(str_value, it->equ_conf.str_standard);

				memset(ch_value, 0, 64);
				if (1 != it->equ_conf.n_type)
				{
					sprintf_s(ch_value, "%d", it->data_struct.n_work);
					str_value.assign(ch_value);
					m_pProducer->SendKafaData(str_value, it->equ_conf.str_work);
				}

				memset(ch_value, 0, 64);
				sprintf_s(ch_value, "%f", it->data_struct.f_immediate);
				str_value.assign(ch_value);
				m_pProducer->SendKafaData(str_value, it->equ_conf.str_immediate);

				if (1 == it->equ_conf.n_type)
				{
					char ch_result[256] = { 0 };
					sprintf_s(ch_result, "IP:%s,press:%f,temperature:%f,standard:%f,immediate:%f,time:%d,it:time:%d",
						it->equ_conf.str_ip.c_str(), data_struct.f_press, data_struct.f_temperature, data_struct.f_standard,
						data_struct.f_immediate, data_struct.n_update_time, it->data_struct.n_update_time);
					log4Util::Debug(ch_result);
					cout << "len:" << strlen(ch_result) << ch_result << endl;
				}
				else
				{
					char ch_result[256] = { 0 };
					sprintf_s(ch_result, "IP:%s,press:%f,temperature:%f,standard:%d,work:%d,immediate:%f,time:%d,it:time:%d",
						it->equ_conf.str_ip.c_str(), data_struct.f_press, data_struct.f_temperature, data_struct.n_standard,
						data_struct.n_work, data_struct.f_immediate, data_struct.n_update_time, it->data_struct.n_update_time);
					log4Util::Debug(ch_result);
					cout<<"len:"<<strlen(ch_result) << ch_result << endl;
				}
			}
			else if((time(NULL) - it->data_struct.n_update_time > 180) && 
				(0 != it->data_struct.n_update_time))
			{
				// 如果超过3分钟没有更新数据，可能出现线程挂起情况，结束老线程，启动新线程
				//it->my_client->StartRun();
				char ch_log[256] = { 0 };
				sprintf_s(ch_log, "ReStart thread:%s,flag:%d.", it->equ_conf.str_ip.c_str(), it->my_client->m_con_flag);
				log4Util::Info(ch_log);
				// 重置下接受数据时间，防止还没收到服务器数据就重新开启线程
				// 因为是server发送数据1分钟之内发送都算正常
				it->data_struct.n_update_time = time(NULL);
				it->my_client->ReStartMyThread();
				Sleep(10000);	// 结束线程后增加10秒等待时间
			}
			else if ((false == it->my_client->m_con_flag))
			{
				// 如果线程中连接不成功或者线程结束，重新启动新线程
				char ch_log[256] = { 0 };
				sprintf_s(ch_log, "Reconnect:%s,port:%d.", it->equ_conf.str_ip.c_str(),it->equ_conf.port);
				log4Util::Info(ch_log);
				it->my_client->StartMyThread();
				Sleep(10000);	// 结束线程后增加10秒等待时间
			}
			//cout << "it:time:" << it->data_struct.n_update_time << " time(null)" << time(NULL) << endl;
		}
		Sleep(1000);
	}
	
	system("pause");

	log4Util::close();
	return 0;
}