#pragma once
#include <string>
#include <iostream>
#include <windows.h>

typedef union STRTOFLOAT
{
	BYTE by_data[4];
	float f_data;
	int  n_data;
}StrToFloat;

typedef struct MYDATETYPE
{
	float f_data;
	int n_data;
	MYDATETYPE()
	{
		f_data = 0;
		n_data = 0;
	}
}MyData;

class MyString
{
public:
	MyString();
	~MyString();
public:
	void  string_replace(std::string &str_src, const std::string &replace_src, const std::string &replace_dst);
	MyData str_to_float(std::string str_src, int type = 0);
	int ReduceSame(std::string str_src);
	double AnalySum(std::string str_src);
private:

};