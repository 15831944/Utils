#include <StringConv.h>
#include <string>
#include <cstring>
//#undef WIN32
#ifdef WIN32
    #include <windows.h>
#else
    #include <cconv/cconv.h>
#endif

#ifdef WIN32
#define CDT_CP_GBK			936
#define CDT_CP_BIG5			950
#define CDT_CP_UTF8			65001
#else
#define CDT_CP_GBK_GB2312		    "GB2312"
#define CDT_CP_GBK_GBL				"GB18030"
#define CDT_CP_GBK					"GBK"
#define CDT_CP_BIG5					"BIG-5"
#define CDT_CP_UTF8					"UTF-8"
#define CDT_CP_UNICODE				"UNICODE"
#endif

enum SWAP_SIZE
{
    SWAP_SIZE_U2G = 1, 
    SWAP_SIZE_G2U = 2,
    SWAP_SIZE_U2B = 1, 
    SWAP_SIZE_B2U = 2,
    SWAP_SIZE_U2UTF8 = 3, 
    SWAP_SIZE_UTF82U = 1,
    SWAP_SIZE_G2B = 1, 
    SWAP_SIZE_B2G = 1,
    SWAP_SIZE_S2T = 1, 
    SWAP_SIZE_T2S = 1,
};


namespace StringUtil
{
	CStringConv::CStringConv(const CStringConv& Right)
	{
		this->m_nSize = 0;
		this->m_pData = NULL;
		CopyFrom(Right);
	}

	CStringConv& CStringConv::operator = (const CStringConv& Right)
	{
		if (this == &Right)
			return *this;
		CopyFrom(Right);
		return *this;
	}

	CStringConv::~CStringConv()
	{
		if (m_pData)
		{
			delete[] m_pData;
			m_pData = NULL;
		}
	}

    bool CStringConv::Prepare(char* input, int nInLen, int nSwapSize)
	{
        //除了UNICODE之外所有的字符串都是已'\0'结束
        //nStrZero定义了UNICODE'\0'长度以便兼容所有字符串
        static int nStrZero = 2;

		try
		{
            if (!input || (nInLen <= 0) || (nSwapSize <= 0))
                return false;

            int nNewSize = (nInLen * nSwapSize) + nStrZero;
            if (nNewSize > m_nSize)
			{
				if (m_pData)
				{
					delete[] m_pData;
					m_pData = NULL;
				}
                m_nSize = nNewSize;
				m_pData = new char[m_nSize];
			}
			memset(m_pData, 0, m_nSize);
			return true;
		}
		catch (...)
		{
			m_nSize = 0;
			return false;
		}
	}

	void CStringConv::CopyFrom(const CStringConv& Right)
	{
		if (this->m_nSize < Right.m_nSize)
		{
			m_nSize = Right.m_nSize;
			if (m_pData)
			{
				delete[] m_pData;
				m_pData = NULL;
			}	
			m_pData = new char[this->m_nSize];
			memcpy(m_pData, Right.m_pData, m_nSize);
		}
		else
		{
			if (Right.m_pData)
			{
				memset(this->m_pData, 0, this->m_nSize);
				memcpy(m_pData, Right.m_pData, Right.m_nSize);
			}
		}
	}

	int CStringConv::U2G(char* input, int nInLen)
	{
        return Prepare(input, nInLen, SWAP_SIZE_U2G) ? U2G(input, nInLen, m_pData, m_nSize) : -1;
	}

	int CStringConv::G2U(char* input, int nInLen)
	{
        return Prepare(input, nInLen, SWAP_SIZE_G2U) ? G2U(input, nInLen, m_pData, m_nSize) : -1;
	}

	int CStringConv::U2B(char* input, int nInLen)
	{
        return Prepare(input, nInLen, SWAP_SIZE_U2B) ? U2B(input, nInLen, m_pData, m_nSize) : -1;
	}
	int CStringConv::B2U(char* input, int nInLen)
	{
        return Prepare(input, nInLen, SWAP_SIZE_B2U) ? B2U(input, nInLen, m_pData, m_nSize) : -1;
	}

	//UNICODE to UTF8
	int CStringConv::U2UTF8(char* input, int nInLen)
	{
        return Prepare(input, nInLen, SWAP_SIZE_U2UTF8) ? U2UTF8(input, nInLen, m_pData, m_nSize) : -1;
	}
	int CStringConv::UTF82U(char* input, int nInLen)
	{
        return Prepare(input, nInLen, SWAP_SIZE_UTF82U) ? UTF82U(input, nInLen, m_pData, m_nSize) : -1;
	}

	int CStringConv::G2B(char* input, int nInLen)
	{
        return Prepare(input, nInLen, SWAP_SIZE_G2B) ? G2B(input, nInLen, m_pData, m_nSize) : -1;
	}
	int CStringConv::B2G(char* input, int nInLen)
	{
        return Prepare(input, nInLen, SWAP_SIZE_B2G) ? B2G(input, nInLen, m_pData, m_nSize) : -1;
	}

	int CStringConv::S2T(char* input, int nInLen)
	{
        return Prepare(input, nInLen, SWAP_SIZE_S2T) ? S2T(input, nInLen, m_pData, m_nSize) : -1;
	}

	int CStringConv::T2S(char* input, int nInLen)
	{
        return Prepare(input, nInLen, SWAP_SIZE_T2S) ? T2S(input, nInLen, m_pData, m_nSize) : -1;
	}

#ifdef WIN32
	int W2M(UINT nCodepage, WCHAR* pWs, int nWlen, CHAR* pMs, int nMlen)
	{
		int nOutSize = WideCharToMultiByte(nCodepage, 0, pWs, nWlen, pMs, nMlen, 0, 0);
		pMs[nOutSize] = '\0';
		return nOutSize;
	}

	int M2W(UINT nCodepage, CHAR* pMs, int nMlen, WCHAR* pWs, int nWlen)
	{
		int nOutSize = MultiByteToWideChar(nCodepage, 0, pMs, nMlen, pWs, nWlen);
		pWs[nOutSize] = L'\0';
		return nOutSize;
	}
	//GBK （含繁简体）转 UNICODE
    int CStringConv::G2U(char* input, int nInLen, char* output, int nOutLen)
	{
		return M2W(CDT_CP_GBK, input, nInLen, (WCHAR*)output, nOutLen / sizeof(WCHAR));
	}
	//UNICODE转GBK（含繁简体）
    int CStringConv::U2G(char* input, int nInLen, char* output, int nOutLen)
	{
		return W2M(CDT_CP_GBK, (WCHAR*)input, nInLen / sizeof(WCHAR), output, nOutLen);
	}
	//BIG5转UNICODE
    int CStringConv::B2U(char* input, int nInLen, char* output, int nOutLen)
	{
		return M2W(CDT_CP_BIG5, input, nInLen, (WCHAR*)output, nOutLen / sizeof(WCHAR));
	}
	//UNICODE转BIG5
    int CStringConv::U2B(char* input, int nInLen, char* output, int nOutLen)
	{
		return W2M(CDT_CP_BIG5, (WCHAR*)input, nInLen / sizeof(WCHAR), output, nOutLen);
	}
	//UNICODE 转UTF8
    int CStringConv::U2UTF8(char* input, int nInLen, char* output, int nOutLen)
	{
		return W2M(CDT_CP_UTF8, (WCHAR*)input, nInLen / sizeof(WCHAR), output, nOutLen);
	}
	//UTF8 转 UNICODE 
    int CStringConv::UTF82U(char* input, int nInLen, char* output, int nOutLen)
	{
		return M2W(CDT_CP_UTF8, input, nInLen, (WCHAR*)output, nOutLen / sizeof(WCHAR));
	}
	
    int CStringConv::S2T(char* input, int nInLen, char* output, int nOutLen)
	{
		WORD wLangID = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
		LCID Locale = MAKELCID(wLangID, SORT_CHINESE_PRCP);
		return LCMapStringA(Locale, LCMAP_TRADITIONAL_CHINESE, input, nInLen, output, nOutLen);
	}

    int CStringConv::T2S(char* input, int nInLen, char* output, int nOutLen)
	{
		WORD wLangID = MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
		LCID Locale = MAKELCID(wLangID, SORT_CHINESE_PRCP);
		return LCMapStringA(Locale, LCMAP_SIMPLIFIED_CHINESE, input, nInLen, output, nOutLen);
	}

	//GB2312(简体)转BIG5繁体
    int CStringConv::G2B(char* input, int nInLen, char* output, int nOutLen)
	{
		CHAR* pGBKBuff = NULL;
		CHAR* pUniBuff = NULL;
		int nRet = 0;
		do
		{
			pGBKBuff = new CHAR[nInLen + 1];
			if (!pGBKBuff)
				break;
			memset(pGBKBuff, 0, nInLen + 1);
			if (S2T(input, nInLen, pGBKBuff, nInLen) <= 0)
				break;
			int nNewLen = (strlen(pGBKBuff) + 1) * sizeof(WCHAR);
			CHAR* pUniBuff = new CHAR[nNewLen];
			if (!pUniBuff)
				break;
			memset(pUniBuff, 0, nNewLen);
			if (G2U(pGBKBuff, strlen(pGBKBuff), pUniBuff, nNewLen) <= 0)
				break;
			WCHAR* pTmpBuff = (WCHAR*)pUniBuff;
			nRet = U2B(pUniBuff, wcslen(pTmpBuff)*sizeof(WCHAR), output, nOutLen);
			if (pUniBuff)
				delete[] pUniBuff;
		} while (0);

		if (pGBKBuff) delete[] pGBKBuff;
		if (pUniBuff) delete[] pUniBuff;
		return nRet;
	}

	//BIG5繁体 转GB2312(简体)
    int CStringConv::B2G(char* input, int nInLen, char* output, int nOutLen)
	{
		CHAR* pGBKBuff = NULL;
		CHAR* pUniBuff = NULL;
		int nRet = 0;
		do
		{
			int nNewLen = (nInLen + 1) * sizeof(WCHAR);
			pUniBuff = new CHAR[nNewLen];
			if (!pUniBuff)
				break;

			memset(pUniBuff, 0, nNewLen);
			if (B2U(input, nInLen, pUniBuff, nNewLen) <= 0)
				break;

			pGBKBuff = new CHAR[nNewLen];
			if (!pGBKBuff)
				break;

			memset(pGBKBuff, 0, nNewLen);
			if (U2G(pUniBuff, nNewLen, pGBKBuff, nNewLen) <= 0)
				break;

			nRet = T2S(pGBKBuff, strlen(pGBKBuff), output, nOutLen);
		} while (0);

		if (pGBKBuff) delete[] pGBKBuff;
		if (pUniBuff) delete[] pUniBuff;
		return nRet;
	}

#else

	//iconv转换实现函数
	int IConvert(const char* fset, const char* tset, char* pIn, size_t nInLen, char* pOut, size_t nOutLen)
	{
		memset(pOut, 0, nOutLen);
		iconv_t _conv = iconv_open(tset, fset);
		if (_conv <= 0)
			return 0;
		size_t conv_size = iconv(_conv, &pIn, &nInLen, &pOut, &nOutLen);
		iconv_close(_conv);
		return -1 == conv_size ? conv_size : nOutLen;
	}

	//cconv转换实现函数
	int CConvert(const char* fset, const char* tset, char* pIn, size_t nInLen, char* pOut, size_t nOutLen)
	{
		memset(pOut, 0, nOutLen);
		cconv_t _conv = cconv_open(tset, fset);
		if (_conv <= 0)
			return 0;
		size_t conv_size = cconv(_conv, &pIn, &nInLen, &pOut, &nOutLen);
		cconv_close(_conv);
		return -1 == conv_size ? conv_size : nOutLen;
	}

	//UNICODE转GBK（含繁简体）
    int CStringConv::U2G(char* input, int nInLen, char* output, int nOutLen)
	{
		return IConvert(CDT_CP_UNICODE, CDT_CP_GBK, input, nInLen, output, nOutLen);
	}

	//GBK （含繁简体）转 UNICODE
    int CStringConv::G2U(char* input, int nInLen, char* output, int nOutLen)
	{
		return IConvert(CDT_CP_GBK, CDT_CP_UNICODE, input, nInLen, output, nOutLen);
	}

	//UNICODE转BIG5
    int CStringConv::U2B(char* input, int nInLen, char* output, int nOutLen)
	{
		char* utf8 = new char[nOutLen*3];
		memset(utf8,0,nOutLen*3);
		IConvert(CDT_CP_UNICODE, CDT_CP_UTF8, input, nInLen, utf8, nOutLen*3);
		int ret = CConvert(CCONV_CODE_UTF, CCONV_CODE_BIG, utf8, nOutLen*3, output, nOutLen); 
		delete [] utf8;
		return ret;
	}

	//BIG5转UNICODE
    int CStringConv::B2U(char* input, int nInLen, char* output, int nOutLen)
	{
		return IConvert(CDT_CP_BIG5, CDT_CP_UNICODE, input, nInLen, output, nOutLen);
	}

	// UNICODE转UTF8
    int CStringConv::U2UTF8(char* input, int nInLen, char* output, int nOutLen)
	{
		return IConvert(CDT_CP_UNICODE, CDT_CP_UTF8, input, nInLen, output, nOutLen);
	}

	//UTF8转UNICODE
    int CStringConv::UTF82U(char* input, int nInLen, char* output, int nOutLen)
	{
		return IConvert(CDT_CP_UTF8, CDT_CP_UNICODE, input, nInLen, output, nOutLen);
	}

	//GB2312(简体)转BIG5繁体
    int CStringConv::G2B(char* input, int nInLen, char* output, int nOutLen)
	{
		return CConvert(CCONV_CODE_GBL, CCONV_CODE_BIG, input, nInLen, output, nOutLen);
	}

	//BIG5转GB2312（简体）  和windos的实现相对应
    int CStringConv::B2G(char* input, int nInLen, char* output, int nOutLen)
	{
		char* gbk = new char[nOutLen];
		memset(gbk,0,nOutLen);
		CConvert(CCONV_CODE_BIG, CCONV_CODE_GBL, input, nInLen, gbk, nOutLen);
		int ret = CConvert(CCONV_CODE_GBL, CCONV_CODE_GHS, gbk, nOutLen, output, nOutLen);
		delete[] gbk;
		return ret;
	}
	//GB18030简体转繁体
    int CStringConv::S2T(char* input, int nInLen, char* output, int nOutLen)
	{
		return CConvert(CCONV_CODE_GBL, CCONV_CODE_GHT, input, nInLen, output, nOutLen);
	}
	//GB18030繁体转简体
    int CStringConv::T2S(char* input, int nInLen, char* output, int nOutLen)
	{
		return CConvert(CCONV_CODE_GBL, CCONV_CODE_GHS, input, nInLen, output, nOutLen);
	}

#endif
}
