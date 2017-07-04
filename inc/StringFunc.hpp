#pragma once

#include <string.h>
#include <string>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
using namespace std;


namespace StringUtil
{
    template<typename R, typename I>
    R lexical_cast(const I& Arg, const R& Def)
    {
        try
        {
            return boost::lexical_cast<R>(Arg);
        }
        catch (...)
        {
            return Def;
        }
    }

    //��Input��Str�и����������Output
    static void Splite(const string& Input, const string& Str, vector<string>&Output)
    {
		Output.clear();
		//����޷��и����
		if (Input.empty() || Str.empty())
			return;
		string::size_type cur_pos = Input.find(Str);
		string::size_type pre_pos = 0;
		string token;
		while (cur_pos != string::npos)
		{
			if (cur_pos&&!(token = Input.substr(pre_pos, cur_pos - pre_pos)).empty())
				Output.push_back(token);
			pre_pos = cur_pos + Str.length();
			cur_pos = Input.find(Str.c_str(), pre_pos, Str.length());
		}
		if (pre_pos != Input.length())
			Output.push_back(Input.substr(pre_pos, Input.length() - pre_pos));
    }

    //����ַ����ұ߲��ɶ��ַ� - �Ƽ�ֱ��ʹ��boost��
	static inline string RightTrim(const string& Input)
    {	
		string input_tr = Input;
		boost::trim_right(input_tr);
		return input_tr;
    }

    //����ַ�����߲��ɶ��ַ� ���� �س� tab �ո�ȵ�  - �Ƽ�ֱ��ʹ��boost��
	static inline string LeftTrim(const string& Input)
    {
		string input_tl = Input;
		boost::trim_left(input_tl);
		return input_tl;
    }

    //����ַ������߲��ɶ��ַ� - �Ƽ�ֱ��ʹ��boost��
	static inline  string Trim(const string& Input)
    {
		string input_tb = Input;
		boost::trim(input_tb);
		return input_tb;
    }

    //���ַ���Input�е�����Str1�滻��Str2 - �Ƽ�ֱ��ʹ��boost��
	static inline string Replace(const string& Input, const string& Str1, const string& Str2)
    {
		string input_r = Input;
		boost::replace_all(input_r, Str1, Str2);
		return input_r;
    }

    //���ַ����������ַ�תΪ��д - �Ƽ�ֱ��ʹ��boost��
	static inline string UpperCase(const string& Input)
    {
		string input_tu = Input;
		boost::to_upper(input_tu);
		return input_tu;
    }

    //���ַ����������ַ�תΪСд - �Ƽ�ֱ��ʹ��boost��
	static inline string LowerCase(const string& Input)
    {
		string input_tl = Input;
		boost::to_lower(input_tl);
		return input_tl;
    }
    //�Ƚ��ַ����Ƿ���ȣ�bCaseInsensitiveΪtrue��ʾ��Сд����
    static inline bool Equal(const char* pStr1, const char* pStr2, bool bCaseInsensitive = true)
    {
        if (bCaseInsensitive)
        {
#ifdef __linux__
#define stricmp strcasecmp
#elif WIN32
#define stricmp _stricmp
#endif
            return (0 == stricmp(pStr1, pStr2));
        }
        return (0 == strcmp(pStr1, pStr2));
    }

}
