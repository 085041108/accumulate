#include "MyClient.h"


void MyCreateThread(LPVOID param)
{
	MyClient * my_client = (MyClient *)param;
	my_client->StartRun();
	char ch_log[64] = { 0 };
	sprintf_s(ch_log, "MyCreateThread:%d,flag:%d.", (int)(my_client->m_h_thread),my_client->m_con_flag);
	log4Util::Debug(ch_log);
	CloseHandle(my_client->m_h_thread);
	int a = 0;
}

MyClient::MyClient(int port, string str_ip, Mutex* pmut_lock)
{
	m_sock_client = INVALID_SOCKET;
	m_port = port;
	m_ip = str_ip;
	m_thread_id = 0;
	m_pmut_lock = pmut_lock;
	memset(m_log_msg, 0, LOG_LEN);
	m_str_rec.assign("");
	m_con_flag = true;

	m_f_press = 0;
	m_f_temperature = 0;
	m_n_standard = 0;
	m_f_standard = 0;
	m_n_work = 0;
	m_f_immediate = 0;
	m_update_time = 0;
	m_get_flag = true;
	m_type = 0;
}

MyClient::~MyClient()
{
	
}

int MyClient::CloseSocket()
{
	if (INVALID_SOCKET != m_sock_client)
	{
		char ch_log[64] = { 0 };
		sprintf_s(ch_log, "CloseSocket:%d.", (unsigned int)(m_sock_client));
		log4Util::Debug(ch_log);
		::closesocket(m_sock_client);
		m_sock_client = INVALID_SOCKET;
	}
	return API_SUCC;
}

int MyClient::Init()
{
	// 如果没有关闭需要先关闭socket再创建
	CloseSocket();

	//创建套接字用于与服务器通信
	m_sock_client = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_sock_client == INVALID_SOCKET)
	{
		memset(m_log_msg, 0, LOG_LEN);
		sprintf_s(m_log_msg, "Faild socket,ThreadId:%d.", m_thread_id);
		log4Util::Error(m_log_msg);
		m_con_flag = false;
		return API_ERROR;
	}

	//绑定服务端的IP和端口
	m_sock_server.sin_family = AF_INET;
	m_sock_server.sin_port = htons(m_port); //服务端端口
	//addrSrv.sin_addr.s_addr = inet_addr("10.1.118.45");
	//InetPton(AF_INET, TEXT("10.1.118.45"), &clientAddr.sin_addr.s_addr);
	//addrSrv.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");  //这里填服务端Ip地址，没有联网用本地“127.0.0.1”
	inet_pton(AF_INET, m_ip.c_str(), (void*)&m_sock_server.sin_addr.S_un.S_addr);

	return API_SUCC;
}

int MyClient::Connect()
{
	//连接服务端
	if (::connect(m_sock_client, (sockaddr*)&m_sock_server, sizeof(m_sock_server)) == -1)
	{
		memset(m_log_msg, 0, LOG_LEN);
		sprintf_s(m_log_msg, "Faild connect,ThreadId:%d.", m_thread_id);
		log4Util::Error(m_log_msg);
		m_con_flag = false;
		return API_ERROR;
	}
	m_con_flag = true;
	return API_SUCC;
}

int MyClient::RecvData()
{
	char cBuff[REC_LEN] = { 0 };
	int iRecv = 0;
	try
	{
		iRecv = ::recv(m_sock_client, cBuff, REC_LEN, 0);
	}
	catch (const std::exception&)
	{
		int a = 0;
	}
	if (iRecv >= REC_LEN)
	{
		memset(m_log_msg, 0, LOG_LEN);
		sprintf_s(m_log_msg, "over the buf length,please check：%d,ThreadId:%d.", iRecv, m_thread_id);
		log4Util::Error(m_log_msg);
	}
	else if (iRecv > 0)
	{
		cBuff[iRecv] = '\0';
		//memset(m_log_msg, 0, LOG_LEN);
		//sprintf_s(m_log_msg, "receive data:%s,ThreadId:%d.", cBuff, m_thread_id);
		//log4Util::Debug(m_log_msg);

		string str_rec = HexToStr(cBuff, iRecv);
		//cout << iRecv << endl;
		// 有出现单包分多次发送的情况，需组合后再解析
		int cc_find = str_rec.find("CC");
		int ee_find = str_rec.find("EE");
		if ( (str_rec.length() >= 196) || 
			((-1 != cc_find) && (-1 != ee_find) && (ee_find - cc_find > 10)/*170特殊处理*/ ) )
		{
			// cout << str_rec << " len:" << str_rec.length() << endl;
			Analyse(str_rec);
		}
		else
		{
			if (1 != m_type)
			{
				m_str_rec.append(str_rec);
				if ( m_str_rec.length() >= 196 )
				{
					Analyse(m_str_rec);
					m_str_rec.assign("");
				}
			}
			else
			{
				if (-1 != cc_find)
				{
					m_str_rec.append(str_rec.substr(0, cc_find));
					Analyse(m_str_rec);
					m_str_rec.assign(str_rec.substr(cc_find));
				}
				else
				{
					m_str_rec.append(str_rec);
				}
			}
		}
		return API_SUCC;
	}
	else
	{
		memset(m_log_msg, 0, LOG_LEN);
		sprintf_s(m_log_msg, "Error code %d,ThreadId:%d.", iRecv, m_thread_id);
		log4Util::Error(m_log_msg);
	}
	
	return API_ERROR;
}

int MyClient::StartRun()
{
	memset(m_log_msg, 0, LOG_LEN);
	sprintf_s(m_log_msg, "Start receive,ThreadId:%d.", m_thread_id);
	log4Util::Info(m_log_msg);

	if (API_ERROR == Init())
	{
		return API_ERROR;
	}
	if (API_SUCC == Connect())
	{
		while (true)
		{
			//for (int i = 0; i < 100; i++)
			//{
			//	CLock cLock(*m_pmut_lock);
			//	m_update_time = i;
			//	cout << /*"线程" << m_thread_id << "线程同步测试!"*/"thread"<<m_update_time<<"lock" << endl;
			//}
			if (API_ERROR == RecvData())
			{
				// 如果recv函数返回错误，则结束线程，
				m_con_flag = false;
				CloseSocket();
				return API_ERROR;
			}
		}
	}
	else
	{
		return API_ERROR;
	}

	return API_SUCC;
}

string MyClient::HexToStr(char* p_hex, int len)
{
	//13 57 EC 60
	std::string str_byte = "";
	char ch_str[4] = { 0 };
	for (int i = 0; i < len; i++)
	{
		sprintf_s(ch_str, "%02X", (unsigned char)p_hex[i]);
		str_byte.append(ch_str);
		str_byte.append(" ");
	}
	return str_byte;
}

int MyClient::Analyse(string str_rec)
{
	/*std::string str_rec;
	str_rec.assign(p_rec);*/
	//std::cout << str_rec << "len:" << str_rec.length() << std::endl;
	if (1 != m_type)
	{
		if (str_rec.length() >= 194)
		{
			MyString my_string;
			my_string.string_replace(str_rec, " ", "");
			int star_addr = str_rec.find("E0AE0601");
			std::string str_temp = str_rec.substr(star_addr + 18, 8); // DD 9A 29 40
			float f_press = my_string.str_to_float(str_temp).f_data;
			str_temp = str_rec.substr(star_addr + 26, 8);
			float f_temperature = my_string.str_to_float(str_temp).f_data;
			str_temp = str_rec.substr(star_addr + 34, 8);
			int n_standard = my_string.str_to_float(str_temp, 1).n_data;
			float f_standard = my_string.str_to_float(str_temp, 1).n_data;
			str_temp = str_rec.substr(star_addr + 50, 8);
			int n_work = my_string.str_to_float(str_temp, 1).n_data;
			str_temp = str_rec.substr(star_addr + 66, 8);
			float f_immediate = my_string.str_to_float(str_temp).f_data;

			CLock cLock(*m_pmut_lock);
			m_get_flag = false;
			m_f_press = f_press;
			m_f_temperature = f_temperature;
			m_n_standard = n_standard;
			m_f_standard = f_standard;
			m_n_work = n_work;
			m_f_immediate = f_immediate;
			m_update_time = time(NULL);
		}
		else
		{
			string str_log;
			str_log.assign(m_ip);
			str_log.append(" Abandon the package DAM:");
			str_log.append(str_rec);
			log4Util::Info(str_log.c_str());
		}
	}
	else
	{
		if ((-1 != str_rec.find("CC")) && (-1 != str_rec.find("EE")))
		{
			// 170 传输问题增加报文长度限制
			int cc_find = str_rec.find("CC");
			int ee_find = str_rec.rfind("EE");
			//cout << "*******" << str_rec << endl;
			if (105 != ee_find - cc_find)
			{
				string str_log;
				str_log.assign(m_ip);
				char ch_len[64] = { 0 };
				sprintf_s(ch_len, " cc:%d,ee:%d", cc_find, ee_find);
				str_log.append(ch_len);
				str_log.append(" Len is wrong TX:");
				str_log.append(str_rec);
				log4Util::Info(str_log.c_str());
				return API_ERROR;
			}

			// log4Util::Info(str_rec.c_str());
			MyString my_string;
			my_string.string_replace(str_rec, " ", "");
			int star_addr = str_rec.find("CC");
			std::string str_temp = str_rec.substr(star_addr + 24, 8);
			m_f_immediate = my_string.AnalySum(str_temp);

			/*string str_log;
			str_log.assign(m_ip);
			char ch_len[64] = { 0 };
			sprintf_s(ch_len, " cc:%d,ee:%d,start:%d", cc_find, ee_find, star_addr);
			str_log.append(ch_len);
			str_log.append(" Len is wrong TX:");
			str_log.append(str_rec);
			log4Util::Info(str_log.c_str());*/

			str_temp = str_rec.substr(star_addr + 32, 4);
			int n_base = atoi(str_temp.c_str());
			int n_mult = n_base * 1000000;
			double d_res = my_string.AnalySum(str_rec.substr(star_addr + 36, 8));
			m_f_standard = d_res + n_mult;

			str_temp = str_rec.substr(star_addr + 44, 8);
			m_f_temperature = my_string.AnalySum(str_temp);

			str_temp = str_rec.substr(star_addr + 52, 8);
			m_f_press = my_string.AnalySum(str_temp);
			

			// 数据传输超出范围时舍弃
			if ((m_f_immediate > 1000000000 || m_f_immediate < -1000000000) || 
				(m_f_standard > 10000000000 || m_f_standard < -1000000000) ||
				(m_f_temperature > 1000000000 || m_f_temperature < -1000000000) || 
				(m_f_press > 1000000000 || m_f_press < -1000000000))
			{
				char ch_result[256] = { 0 };
				sprintf_s(ch_result, "Error data IP:%s,press:%f,temperature:%f,standard:%f,immediate:%f",
					m_ip.c_str(), m_f_press, m_f_temperature, m_f_standard,m_f_immediate);
				log4Util::Info(ch_result);
				log4Util::Info(str_rec.c_str());
				return API_ERROR;
			}
			m_update_time = time(NULL);
		}
		else
		{
			string str_log;
			str_log.assign(m_ip);
			str_log.append(" Abandon the package TX:");
			str_log.append(str_rec);
			log4Util::Info(str_log.c_str());
		}
	}
	
	//cout << "主线程调用thread" << m_thread_id << "thread sys!" << endl;
	return API_SUCC;
}

int MyClient::StartMyThread()
{
	m_h_thread = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(MyCreateThread), this, 0, &m_thread_id);
	return API_SUCC;
}

int MyClient::ReStartMyThread()
{
	// 如果没有关闭需要先关闭socket再创建
	CloseSocket();
	char ch_log[64] = { 0 };
	//sprintf_s(ch_log, "TerminateThread before:%d.", (int)(m_h_thread));
	//log4Util::Debug(ch_log);

	// 实际测试中不加sleep会出现挂起现象
	Sleep(1000);
	TerminateThread(m_h_thread, 0);
	Sleep(1000);

	//memset(ch_log, 0, 64);
	//sprintf_s(ch_log, "TerminateThread after:%d.", (int)(m_h_thread));
	//log4Util::Debug(ch_log);

	m_h_thread = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(MyCreateThread), this, 0, &m_thread_id);

	memset(ch_log, 0, 64);
	sprintf_s(ch_log, "CreateThread after:%d.", (int)(m_h_thread));
	log4Util::Debug(ch_log);
	return API_SUCC;
}

int MyClient::GetValue(DataStruct& data_struct)
{
	//CLock cLock(*m_pmut_lock);	// 不做修改，不加锁
	data_struct.f_press = m_f_press;
	data_struct.f_temperature = m_f_temperature;
	data_struct.n_standard = m_n_standard;
	data_struct.f_standard = m_f_standard;
	data_struct.n_work = m_n_work;
	data_struct.f_immediate = m_f_immediate;
	data_struct.n_update_time = m_update_time;
	return API_SUCC;
}
