#pragma once
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <csignal>
#include <cstring>
#include <fstream>
#include <windows.h>
#include <time.h>

//#include <getopt.h>

#include "../include/rdkafkacpp.h"
#include "../Clog4Util.h"

#define INTERVAL_CHANGE  1000	// 入库频率先设成1秒
#define INTERVAL_QUERY  300000	// 入库频率先设成5分钟

static bool run = true;

static void sigterm(int sig) {
	run = false;
}

class ExampleDeliveryReportCb : public RdKafka::DeliveryReportCb {
public:
	void dr_cb(RdKafka::Message &message) {
		std::cout << "Message delivery for (" << message.len() << " bytes): " <<
			message.errstr() << std::endl;
		if (message.key())
			std::cout << "Key: " << *(message.key()) << ";" << std::endl;
	}
};

class ExampleEventCb : public RdKafka::EventCb {
public:
	void event_cb(RdKafka::Event &event) {
		switch (event.type())
		{
		case RdKafka::Event::EVENT_ERROR:
			std::cerr << "ERROR (" << RdKafka::err2str(event.err()) << "): " <<
				event.str() << std::endl;
			if (event.err() == RdKafka::ERR__ALL_BROKERS_DOWN)
				run = false;
			break;

		case RdKafka::Event::EVENT_STATS:
			std::cerr << "\"STATS\": " << event.str() << std::endl;
			break;

		case RdKafka::Event::EVENT_LOG:
			fprintf(stderr, "LOG-%i-%s: %s\n",
				event.severity(), event.fac().c_str(), event.str().c_str());
			break;

		default:
			std::cerr << "EVENT " << event.type() <<
				" (" << RdKafka::err2str(event.err()) << "): " <<
				event.str() << std::endl;
			break;
		}
	}
};


class MyProducer 
{
private:
	MyProducer();
	~MyProducer();

	static MyProducer* instance;
public:
	static MyProducer* getInstance() {
		if (instance == NULL){
			instance = new MyProducer();
			//cout << "it is first" << endl;
		}else{
			//cout << "it is not first" << endl;
		}
		return instance;
	}
public:
	void MakeProducer(std::string content);
	void SendKafaData(std::string str_value, std::string item_des);
	void string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst);
public:
	std::string m_metric_str;
	std::string m_str_tags;
	int m_inter_query;
	int m_inter_change;
private:
	std::string m_brokers;
	std::string m_errstr;
	std::string m_topic_str;
	int32_t m_partition;
	RdKafka::Conf *m_pConf;
	RdKafka::Conf *m_pTconf;
	RdKafka::Producer *m_pProducer;
	RdKafka::Topic *m_pTopic;
	ExampleDeliveryReportCb m_ex_dr_cb;
	ExampleEventCb m_ex_event_cb;
	char m_strLog_[1024];
};