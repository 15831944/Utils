/***********************************************************************
模块名	: Utils
文件名	: UniqueCtx.h
作者		: DW
创建日期	: 2017/1/9
备注		: snowflake分布式唯一id算法实现（非多线程安全）
**************************************************************************/
#pragma once

#include "export.h"
#include "exception.h"

// 0 - 11[12] : 自增码(每毫秒内最多支持生成4096个唯一id)
//12 - 21[10] : 机器码(取值范围0 - 1023) 最多支持1024个设备同时生成唯一id
//22 - 63[42] : 时间位(毫秒)

namespace Utils
{
    typedef unsigned long long __uuint64;
    typedef unsigned long long __uuint42;
    typedef unsigned short __uuint12;
    typedef unsigned short __uuint10;

    class UTILT_API CUniqueCtx
    {
    public:
        CUniqueCtx(__uuint10 nMachine);
        __uuint64 GetUniqueId();

    private:
        void Prepare();
        static __uuint64 _GetLocalTime64();
        static const __uuint64 UUINT42_MASK = 0x3ffffffffff;  //时间位掩码
        static const __uuint64 UUINT12_MASK = 0xfff;  //序列位掩码
        static const __uuint64 UUINT10_MASK = 0x3ff; //机器位掩码

        static const __uuint64 TIME_OFFSET = 22; //时间位偏移量
        static const __uuint64 MACH_OFFSET = 12; //机器位偏移量
        static const __uuint64 SEQ_OFFSET = 0; //序列位偏移量

    private:
        __uuint64 m_tmCtx;
        __uuint10 m_nMachine;
        __uuint12 m_nSeqCode;

    private:
        CUniqueCtx(const CUniqueCtx& rhs);
        CUniqueCtx& operator =(const CUniqueCtx& rhs);
    };
}

// example:
// int main(int argc, char* argv[])
// {
//     using namespace Utils;
//     try
//     {
//         CUniqueCtx nCtx(512);
//         __uuint64 id1 = nCtx.GetUniqueId();
//         __uuint64 id2 = nCtx.GetUniqueId();
//         __uuint64 id3 = nCtx.GetUniqueId();
//         assert(id1 != id2);
//         assert(id2 != id3);
//         assert(id1 != id3);
//     }
//     catch (std::exception& e)
//     {
//         cout << "error" << endl;
//     }
// }



