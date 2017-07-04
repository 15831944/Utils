/***********************************************************************
ģ����	: Utils
�ļ���	: UniqueCtx.cpp
����		: DW
��������	: 2017/1/9
��ע		: snowflake�ֲ�ʽΨһid�㷨ʵ�֣��Ƕ��̰߳�ȫ��
**************************************************************************/

#include "workexception.hpp"
#include <time.h>
#include "UniqueCtx.h"
using namespace std;

#ifdef WIN32
#   include <Windows.h>
#else 
#   include <sys/time.h>
#endif

namespace Utils
{
    __uuint64 CUniqueCtx::_GetLocalTime64()
    {
#ifdef WIN32
        time_t clock = 0;
        struct tm tm = { 0 };
        SYSTEMTIME wtm = { 0 };
        GetLocalTime(&wtm);
        tm.tm_year = wtm.wYear - 1900;
        tm.tm_mon = wtm.wMonth - 1;
        tm.tm_mday = wtm.wDay;
        tm.tm_hour = wtm.wHour;
        tm.tm_min = wtm.wMinute;
        tm.tm_sec = wtm.wSecond;
        tm.tm_isdst = -1;
        clock = mktime(&tm);
        __uuint64 sec64 = __uuint64(clock) * 1000;
        __uuint64 usec64 = wtm.wMilliseconds;
#else
        timeval tv = { 0 };
        gettimeofday(&tv, 0);
        __uuint64 sec64 = __uuint64(tv.tv_sec) * 1000;
        __uuint64 usec64 = __uuint64(tv.tv_usec) / 1000;
#endif  
        return sec64 + usec64;
    }

    CUniqueCtx::CUniqueCtx(__uuint10 nMachine)
        : m_tmCtx(0)
        , m_nMachine(nMachine)
        , m_nSeqCode(0)
    {
        if ((m_nMachine & (~UUINT10_MASK)) > 0)
            throw Utils::CWorkException("OUT OF UUINT10_MASK LENGTH");
    }

    void CUniqueCtx::Prepare()
    {
        __uuint64 tmNow = 0;

        do
        {
            //��ȡ��ǰʱ�䣬�����ǰʱ�䳬��ʱ��λ(42λ)���ֵʱ���׳��쳣
            tmNow = _GetLocalTime64();
            if ((tmNow & (~UUINT42_MASK)) > 0)
                throw Utils::CWorkException("OUT OF UUINT42_MASK LENGTH");

            //��ǰʱ���Ƿ�����ϴθ���ʱ��
            if (tmNow > m_tmCtx)
            {
                m_tmCtx = tmNow;
                m_nSeqCode = 0;
                break;
            }
            else
            {
                //��ǰʱ������ϴθ���ʱ�䲢�����кŴﵽ���ֵ - �ڴ˽�������
                if (m_nSeqCode & (~UUINT12_MASK))
                    continue;

                //��ǰʱ������ϴθ���ʱ�䲢�����к�δ�ﵽ���ֵ
                ++m_nSeqCode;
                break;
            }

        } while (1);
    }

    __uuint64 CUniqueCtx::GetUniqueId()
    {
        //������id֮ǰ��鵱ǰ��������ĳһ���������ɵ�id����4095��ʱ
        //��ǰ�߳̽���������״̬(��ѭ��)ֱ��ʱ����뵽��һ�����롣
        Prepare();

        __uuint64 nUniqueId = 0;
        nUniqueId |= (m_tmCtx << TIME_OFFSET);
        nUniqueId |= (m_nMachine << MACH_OFFSET);
        nUniqueId |= (m_nSeqCode << SEQ_OFFSET);
        return nUniqueId;
    }
}


