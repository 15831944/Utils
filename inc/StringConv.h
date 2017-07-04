#pragma once

#include "export.h"

namespace StringUtil
{
    class UTILT_API CStringConv
	{
    public:
        /*
        * �������ƣ� U2G(UNICODE <-> GBK��������)
        * ʹ��˵���� ��������UNICODE�Ǽ������ģ�������δ����
        * ��������� input -- �����ַ���(UNICODE����)
        * ��������� nInLe -- �����ַ�������
        * ��������� output -- ����ַ���GBK(��������)����
        * ��������� nOutLen -- �����ַ�������
        * ���ز����� >0 �����ַ������ȣ� ��������
        */
        static int U2G(char* input, int nInLen, char* output, int nOutLen);
        static int G2U(char* input, int nInLen, char* output, int nOutLen);

        /*
        * �������ƣ� U2B(UNICODE <-> BIG5��������)
        * ʹ��˵���� ��������UNICODE�Ƿ������ģ�������δ����
        * ��������� input -- �����ַ���(UNICODE����)
        * ��������� nInLe -- �����ַ�������
        * ��������� output -- ����ַ���BIG5(��������)����
        * ��������� nOutLen -- �����ַ�������
        * ���ز����� >0 �����ַ������ȣ� ��������
        */
        static int U2B(char* input, int nInLen, char* output, int nOutLen);
        static int B2U(char* input, int nInLen, char* output, int nOutLen);

        /*
        * �������ƣ� U2UTF8(UNICODE <-> UTF8)
        * ʹ��˵���� 
        * ��������� input -- �����ַ���(UNICODE����)
        * ��������� nInLe -- �����ַ�������
        * ��������� output -- ����ַ���UTF8����
        * ��������� nOutLen -- �����ַ�������
        * ���ز����� >0 �����ַ������ȣ� ��������
        */
        static int U2UTF8(char* input, int nInLen, char* output, int nOutLen);
        static int UTF82U(char* input, int nInLen, char* output, int nOutLen);

        /*
        * �������ƣ� G2B(GBK�������� <-> BIG5��������)
        * ʹ��˵���� 
        * ��������� input -- �����ַ���(GBK�������ı���)
        * ��������� nInLe -- �����ַ�������
        * ��������� output -- ����ַ���BIG5��������
        * ��������� nOutLen -- �����ַ�������
        * ���ز����� >0 �����ַ������ȣ� ��������
        */
        static int G2B(char* input, int nInLen, char* output, int nOutLen);
        static int B2G(char* input, int nInLen, char* output, int nOutLen);

        /*
        * �������ƣ� S2T(GBK�������� <-> GBK��������)
        * ʹ��˵���� 
        * ��������� input -- �����ַ���(GBK�������ı���)
        * ��������� nInLe -- �����ַ�������
        * ��������� output -- ����ַ���GBK��������
        * ��������� nOutLen -- �����ַ�������
        * ���ز����� >0 �����ַ������ȣ� ��������
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
