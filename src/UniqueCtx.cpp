/***********************************************************************
模块名	: Utils
文件名	: UniqueCtx.cpp
作者		: DW
创建日期	: 2017/1/9
备注		: snowflake分布式唯一id算法实现（非多线程安全）
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
            //获取当前时间，如果当前时间超过时间位(42位)最大值时将抛出异常
            tmNow = _GetLocalTime64();
            if ((tmNow & (~UUINT42_MASK)) > 0)
                throw Utils::CWorkException("OUT OF UUINT42_MASK LENGTH");

            //当前时间是否大于上次更新时间
            if (tmNow > m_tmCtx)
            {
                m_tmCtx = tmNow;
                m_nSeqCode = 0;
                break;
            }
            else
            {
                //当前时间等于上次更新时间并且序列号达到最大值 - 在此进入自旋
                if (m_nSeqCode & (~UUINT12_MASK))
                    continue;

                //当前时间等于上次更新时间并且序列号未达到最大值
                ++m_nSeqCode;
                break;
            }

        } while (1);
    }

    __uuint64 CUniqueCtx::GetUniqueId()
    {
        //在生成id之前检查当前环境，当某一毫秒内生成的id超过4095个时
        //当前线程将进入自旋状态(死循环)直到时间进入到下一个毫秒。
        Prepare();

        __uuint64 nUniqueId = 0;
        nUniqueId |= (m_tmCtx << TIME_OFFSET);
        nUniqueId |= (m_nMachine << MACH_OFFSET);
        nUniqueId |= (m_nSeqCode << SEQ_OFFSET);
        return nUniqueId;
    }
}


