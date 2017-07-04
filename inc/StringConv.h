#pragma once

#include "export.h"

namespace StringUtil
{
    class UTILT_API CStringConv
	{
    public:
        /*
        * 函数名称： U2G(UNICODE <-> GBK简体中文)
        * 使用说明： 如果输入的UNICODE非简体中文，输出结果未定义
        * 输入参数： input -- 输入字符串(UNICODE编码)
        * 输入参数： nInLe -- 输入字符串长度
        * 输出参数： output -- 输出字符串GBK(简体中文)编码
        * 输出参数： nOutLen -- 输入字符串长度
        * 返回参数： >0 返回字符串长度， 其它错误
        */
        static int U2G(char* input, int nInLen, char* output, int nOutLen);
        static int G2U(char* input, int nInLen, char* output, int nOutLen);

        /*
        * 函数名称： U2B(UNICODE <-> BIG5繁体中文)
        * 使用说明： 如果输入的UNICODE非繁体中文，输出结果未定义
        * 输入参数： input -- 输入字符串(UNICODE编码)
        * 输入参数： nInLe -- 输入字符串长度
        * 输出参数： output -- 输出字符串BIG5(繁体中文)编码
        * 输出参数： nOutLen -- 输入字符串长度
        * 返回参数： >0 返回字符串长度， 其它错误
        */
        static int U2B(char* input, int nInLen, char* output, int nOutLen);
        static int B2U(char* input, int nInLen, char* output, int nOutLen);

        /*
        * 函数名称： U2UTF8(UNICODE <-> UTF8)
        * 使用说明： 
        * 输入参数： input -- 输入字符串(UNICODE编码)
        * 输入参数： nInLe -- 输入字符串长度
        * 输出参数： output -- 输出字符串UTF8编码
        * 输出参数： nOutLen -- 输入字符串长度
        * 返回参数： >0 返回字符串长度， 其它错误
        */
        static int U2UTF8(char* input, int nInLen, char* output, int nOutLen);
        static int UTF82U(char* input, int nInLen, char* output, int nOutLen);

        /*
        * 函数名称： G2B(GBK简体中文 <-> BIG5繁体中文)
        * 使用说明： 
        * 输入参数： input -- 输入字符串(GBK简体中文编码)
        * 输入参数： nInLe -- 输入字符串长度
        * 输出参数： output -- 输出字符串BIG5繁体中文
        * 输出参数： nOutLen -- 输入字符串长度
        * 返回参数： >0 返回字符串长度， 其它错误
        */
        static int G2B(char* input, int nInLen, char* output, int nOutLen);
        static int B2G(char* input, int nInLen, char* output, int nOutLen);

        /*
        * 函数名称： S2T(GBK简体中文 <-> GBK繁体中文)
        * 使用说明： 
        * 输入参数： input -- 输入字符串(GBK简体中文编码)
        * 输入参数： nInLe -- 输入字符串长度
        * 输出参数： output -- 输出字符串GBK繁体中文
        * 输出参数： nOutLen -- 输入字符串长度
        * 返回参数： >0 返回字符串长度， 其它错误
        */
        static int S2T(char* input, int nInLen, char* output, int nOutLen);
        static int T2S(char* input, int nInLen, char* output, int nOutLen);

    public:
        CStringConv() : m_pData(0), m_nSize(0){};
        CStringConv(const CStringConv& Right);
        CStringConv& operator =(const CStringConv& Right);
        ~CStringConv();

        const char* GetData() const { return m_pData; }
        int GetSize() const { return m_nSize; }
        //UNICODE to GB2312(GBK)
		int U2G(char* input, int nInLen);
		int G2U(char* input, int nInLen);

        //UNICODE to BIG5
		int U2B(char* input, int nInLen);
		int B2U(char* input, int nInLen);

		//UNICODE to UTF8
		int U2UTF8(char* input, int nInLen);
		int UTF82U(char* input, int nInLen);

		//GB2312(GBK) to BIG5
		int G2B(char* input, int nInLen);
		int B2G(char* input, int nInLen);

        //Simplified to Traditional
		int S2T(char* input, int nInLen);
		int T2S(char* input, int nInLen);

	private:
		char*   m_pData;
		int     m_nSize;     
        bool Prepare(char* input, int nInLen, int nSwapSize);
        void CopyFrom(const CStringConv& Right);		
	};
}
