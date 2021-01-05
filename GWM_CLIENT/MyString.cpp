#include "MyString.h"

MyString::MyString()
{
}

MyString::~MyString()
{
}

void  MyString::string_replace(std::string &str_src, const std::string &replace_src, const std::string &replace_dst)
{
	std::string::size_type pos = 0;
	std::string::size_type srclen = replace_src.size();
	std::string::size_type dstlen = replace_dst.size();

	while (-1 != (pos = str_src.find(replace_src, pos)))
	{
		str_src.replace(pos, srclen, replace_dst);
		pos += dstlen;
	}
}

/*
DD 9A 29 40  字符串转换为浮点2.6500771
*/
MyData MyString::str_to_float(std::string str_src, int type)
{
	StrToFloat str_to_float;
	char ch_temp[4] = { 0 };
	sscanf_s(str_src.c_str(), "%x", ch_temp);
	//*(int*)(ch_temp) = htonl(*(int*)ch_temp);
	str_to_float.by_data[0] = ch_temp[3];
	str_to_float.by_data[1] = ch_temp[2];
	str_to_float.by_data[2] = ch_temp[1];
	str_to_float.by_data[3] = ch_temp[0];

	MyData my_data;
	if (0 == type)
	{
		my_data.f_data = str_to_float.f_data;
	}
	else
	{
		my_data.n_data = str_to_float.n_data;
	}
	return my_data;
}

/*
// 字符串30 30 30 32 31 33 35 37 3E 3C 36 30，转换成00021357EC60
*/
int MyString::ReduceSame(std::string str_src)
{
	char ch_temp[12] = { 0 };
	std::string str_temp = str_src.substr(0, 8);
	sscanf_s(str_temp.c_str(), "%x", ch_temp);
	str_temp = str_src.substr(8, 8);
	sscanf_s(str_temp.c_str(), "%x", ch_temp + 4);
	str_temp = str_src.substr(16, 8);
	sscanf_s(str_temp.c_str(), "%x", ch_temp + 8);
	*(int*)(ch_temp) = htonl(*(int*)ch_temp);
	*(int*)(ch_temp + 4) = htonl(*(int*)(ch_temp + 4));
	*(int*)(ch_temp + 8) = htonl(*(int*)(ch_temp + 8));
	for (int i = 0; i < 12; i++)
	{
		ch_temp[i] = ch_temp[i] - 0x30;
	}
	str_temp.assign("");
	char ch_new[32] = { 0 };
	for (int i = 0; i < sizeof(ch_temp); i++)
	{
		sprintf_s(ch_new, "%x", (int)ch_temp[i]);
		str_temp.append(ch_new);
	}
	return str_temp.length();
}

/*
// 13 57 EC 60(中间无空格，为了好看增加)转换成十进制360134，分为阶码和尾码解析，负的先要把补码转换成原码
*/
double MyString::AnalySum(std::string str_src)
{
	StrToFloat str_to_float;
	char ch_temp[4] = { 0 };

	memset(ch_temp, 0, sizeof(ch_temp));
	sscanf_s(str_src.c_str(), "%x", ch_temp);
	*(int*)(ch_temp) = htonl(*(int*)ch_temp);

	// 如果是补码则转换成原码
	// 阶码转换
	bool b_flag = false;
	int n_power = 0;
	if (128 == (unsigned char)(ch_temp[0] & 0x80))	// 如果最高位为1，转换后只能是128
	{
		unsigned char ch_dst = (~ch_temp[0]) & 0xFF;
		ch_dst += 0x81;			// 补码是反码加1，再加上符号位
		b_flag = true;
		n_power = ch_dst;
	}
	else
	{
		n_power = ch_temp[0];
	}
	double y_value = 0;
	if (true == b_flag)
	{
		y_value = pow(2, -1 * n_power);
	}
	else
	{
		y_value = pow(2, n_power);
	}


	// 尾数转换
	int x_value = 0;
	if (128 == (unsigned char)(ch_temp[1] & 0x80))	// 如果最高位为1，转换后只能是128
	{
		unsigned char ch_dst = (~ch_temp[1]) & 0xFF;
		ch_dst += 0x80;			// 此处不加1，防止溢出，再加上符号位
		str_to_float.by_data[3] = 0;
		str_to_float.by_data[2] = ch_dst;
		ch_dst = (~ch_temp[2]) & 0xFF;
		str_to_float.by_data[1] = ch_dst;
		ch_dst = (~ch_temp[3]) & 0xFF;
		str_to_float.by_data[0] = ch_dst;
		x_value = str_to_float.n_data + 1;  // 反码到补码的加1放在这里，防止单字节相加溢出

	}
	else
	{
		str_to_float.by_data[3] = 0;
		str_to_float.by_data[2] = ch_temp[1];
		str_to_float.by_data[1] = ch_temp[2];
		str_to_float.by_data[0] = ch_temp[3];
		x_value = str_to_float.n_data;
	}
	double d_result = y_value * x_value / 8388608;
	/*char ch_src = 0xFE;
	int flag = (unsigned char)(ch_src & 0x80);
	unsigned char ch_dst = (~ch_src)&0xFF;
	ch_dst += 0x80;*/
	return d_result;
}
