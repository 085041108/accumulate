#include "SendKafka.h"

MyProducer* MyProducer::instance = NULL;

MyProducer::MyProducer()
{
	char   inBuf[80];
	int ret = GetPrivateProfileString("kafka",
		"ip",
		"Error: failed",
		inBuf,
		80,
		"C:\\configure\\opc.ini");
	std::string str_ip;
	str_ip.assign(inBuf);

	memset(inBuf, 0, 80);
	ret = GetPrivateProfileString("kafka",
		"port",
		"Error: failed",
		inBuf,
		80,
		"C:\\configure\\opc.ini");
	std::string str_port;
	str_port.assign(inBuf);

	memset(inBuf, 0, 80);
	ret = GetPrivateProfileString("kafka",
		"topic",
		"Error: failed",
		inBuf,
		80,
		"C:\\configure\\opc.ini");
	m_topic_str.assign(inBuf);

	memset(inBuf, 0, 80);
	ret = GetPrivateProfileString("kafka",
		"metric",
		"Error: failed",
		inBuf,
		80,
		"C:\\configure\\opc.ini");
	m_metric_str.assign(inBuf);

	memset(inBuf, 0, 80);
	ret = GetPrivateProfileString("kafka",
		"tags",
		"Error: failed",
		inBuf,
		80,
		"C:\\configure\\opc.ini");
	m_str_tags.assign(inBuf);

	m_inter_query = GetPrivateProfileInt("collect",
		"queryinter",
		-1,
		"C:\\configure\\opc.ini");
	m_inter_query = m_inter_query * 1000;
	if (-1 == m_inter_query) {
		m_inter_query = INTERVAL_QUERY; //默认5分钟查一次
	}
	m_inter_change = GetPrivateProfileInt("collect",
		"changeinter",
		-1,
		"C:\\configure\\opc.ini");
	if (-1 == m_inter_change) {
		m_inter_change = INTERVAL_CHANGE; //默认1秒只上报一次变化数据
	}
	m_inter_change = m_inter_change / 1000;

	m_brokers = str_ip;// + ":" + str_port;

	m_partition = RdKafka::Topic::PARTITION_UA;

	m_pConf = RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL);
	m_pTconf = RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC);

	m_pConf->set("bootstrap.servers", m_brokers, m_errstr);

	
	m_pConf->set("event_cb", &m_ex_event_cb, m_errstr);

	signal(SIGINT, sigterm);
	signal(SIGTERM, sigterm);

	
	m_pConf->set("dr_cb", &m_ex_dr_cb, m_errstr);

	m_pProducer = RdKafka::Producer::create(m_pConf, m_errstr);
	if (!m_pProducer) {
		memset(m_strLog_, 0, 1024);
		sprintf_s(m_strLog_, "Failed to create producer:， %s:%d.", __FILE__, __LINE__);
		log4Util::Error(m_strLog_);
		exit(1);
	}

	memset(m_strLog_, 0, 1024);
	sprintf_s(m_strLog_, "%s Created producer.", m_pProducer->name().c_str());
	log4Util::Info(m_strLog_);

	m_pTopic = RdKafka::Topic::create(m_pProducer, m_topic_str,
		m_pTconf, m_errstr);
	if (!m_pTopic) {
		memset(m_strLog_, 0, 1024);
		sprintf_s(m_strLog_, "Failed to create topic:， %s:%d.", __FILE__, __LINE__);
		log4Util::Error(m_strLog_);
		exit(1);
	}

	memset(m_strLog_, 0, 1024);
	sprintf_s(m_strLog_, "Topic is %s  query inter %d", m_topic_str.c_str(), m_inter_query);
	log4Util::Info(m_strLog_);
//	std::cout << "m_pTopic::***" << (int)m_pTopic << "***"<<std::endl;
}

MyProducer::~MyProducer()
{
	run = true;
	// 退出前处理完输出队列中的消息
	while (run && m_pProducer->outq_len() > 0) {
		std::cerr << "Waiting for " << m_pProducer->outq_len() << std::endl;
		m_pProducer->poll(1000);
	}

	delete m_pConf;
	delete m_pTconf;
	delete m_pTopic;
	delete m_pProducer;

	RdKafka::wait_destroyed(5000);
}

void MyProducer::MakeProducer(std::string content)
{
	// add the input kafka time judge
	/*DWORD now_time = GetTickCount();
	int interval = now_time - m_tlast_time;*/

	//std::ofstream myfile;
	//std::string temp;
	//myfile.open("E:\\hello.txt", std::ios::app);
	//if (!myfile.is_open())
	//{
	//	std::cout << "未成功打开文件" << std::endl;
	//}
	//char ch_inter[32] = {};
	//sprintf(ch_inter, "%d***%d***%d\n", now_time, m_tlast_time, interval);
	//myfile << ch_inter;
	//myfile.close();

	//if (interval < INTERVAL_MILLISTIME) {
	//	return;
	//}
	//else {
	//	m_tlast_time = now_time;
	//}

	if (!run) {
		memset(m_strLog_, 0, 1024);
		sprintf_s(m_strLog_, "The kafka server is not run,please check, the broker is %s", m_brokers.c_str());
		log4Util::Error(m_strLog_);
		run = true;
	}

	//  断线了也发送，kafka有缓存，支持断线重发
	if (run) {
//		std::cout << "make procucer55555555  "<<content<<"*m_pTopic***"<<(int)m_pTopic<<"m_par***"<<m_partition<<"m_prod"<<(int)m_pProducer << std::endl;
		RdKafka::ErrorCode resp =
			m_pProducer->produce(m_pTopic, m_partition,
				RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
				const_cast<char *>(content.c_str()), content.size(),
				NULL, NULL);
//		std::cout << "make procucer66666666666" << std::endl;
		if (resp != RdKafka::ERR_NO_ERROR) {
			memset(m_strLog_, 0, 1024);
			sprintf_s(m_strLog_, "%s Produce failed:， %s:%d.", RdKafka::err2str(resp).c_str(), __FILE__, __LINE__);
			log4Util::Error(m_strLog_);
			/*std::cerr << "% Produce failed: " <<
				RdKafka::err2str(resp) << std::endl;*/
		}else {
			/*std::cerr << "% Produced message (" << content.size() << " bytes)" <<
			std::endl;*/
			memset(m_strLog_, 0, 1024);
			sprintf_s(m_strLog_, "%s", content.c_str());
			log4Util::Debug(m_strLog_);
		}
	}
	
//	std::cout << "***producer poll before***" << std::endl;
	m_pProducer->poll(0);	
//	std::cout << "***producer poll after***" << std::endl;
}

void  MyProducer::string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst)
{
	std::string::size_type pos = 0;
	std::string::size_type srclen = strsrc.size();
	std::string::size_type dstlen = strdst.size();

	while (-1 != (pos = strBig.find(strsrc, pos)))
	{
		strBig.replace(pos, srclen, strdst);
		pos += dstlen;
	}
}

//	make kafka type data
void MyProducer::SendKafaData(std::string str_value, std::string item_des)
{
	// 去掉item_des中空格
	std::string str_des = item_des;
	string_replace(str_des, " ", "");		// 把item_des中的空格符号去掉
	std::string kafka_send;
	kafka_send.assign("{\"type\":\"Metric\","
		" \"metric\":");

	std::string item_name;
	item_name.assign("\"");
	item_name.append(m_metric_str);
	item_name.append(".");
	item_name.append(str_des);
	item_name.append("\",");
	kafka_send.append(item_name);


	std::string item_attr = "\"tags\":{\"host\":\"";
	item_attr += m_str_tags;
	item_attr += "\"";
	int nCount = 0;
	std::string str_mutil_tag = str_des;
	string_replace(str_mutil_tag, "-", ".");	// 把采集点中的“-”符号替换成“.”符号
	int nFind = str_mutil_tag.find(".");
	bool nTail = false;
	while (-1 != nFind)
	{
		nTail = true;
		char ch_tag[32] = {};
		sprintf_s(ch_tag, ",\"tag%d\":", nCount++);
		std::string str_temp_tag;
		str_temp_tag.assign(ch_tag);
		std::string str_temp = str_mutil_tag.substr(0, nFind);
		item_attr = item_attr + str_temp_tag + "\"" + str_temp + "\"";
		str_mutil_tag = str_mutil_tag.substr(nFind + 1, str_mutil_tag.length() - nFind);
		nFind = str_mutil_tag.find(".");
	}
	if (nTail)
	{
		char ch_tag[32] = {};
		sprintf_s(ch_tag, ",\"tag%d\":", nCount++);
		std::string str_temp_tag;
		str_temp_tag.assign(ch_tag);
		std::string str_temp = str_mutil_tag;
		item_attr = item_attr + str_temp_tag + "\"" + str_temp + "\"";
	}
	kafka_send.append(item_attr);
	kafka_send.append("},");

	std::string item_time = "\"timestamp\":";
	time_t now_time = time(NULL);
	char ch_timep[32] = {};
	sprintf_s(ch_timep, "%d,", (int)now_time);
	item_time.append(ch_timep);
	kafka_send.append(item_time);

	std::string item_value = "\"value\":";
//	CString cstr_value = FormatItemValue(vtVal);
//	std::string str_value = str_value;
	item_value.append("\"");
	item_value.append(str_value);
	item_value.append("\"");
	item_value.append("}");
	kafka_send.append(item_value);

	MakeProducer(kafka_send);
	//memset(m_strLog_, 0, 1024);
	//sprintf(m_strLog_, "%s", kafka_send.c_str());
	//log4Util::Debug(m_strLog_);

}

