/***********************************************************************
ģ����	: Utils
�ļ���	: UniqueCtx.h
����		: DW
��������	: 2017/1/9
��ע		: snowflake�ֲ�ʽΨһid�㷨ʵ�֣��Ƕ��̰߳�ȫ��
**************************************************************************/
#pragma once

#include "export.h"
#include "exception.h"

// 0 - 11[12] : ������(ÿ���������֧������4096��Ψһid)
//12 - 21[10] : ������(ȡֵ��Χ0 - 1023) ���֧��1024���豸ͬʱ����Ψһid
//22 - 63[42] : ʱ��λ(����)

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
        static const __uuint64 UUINT42_MASK = 0x3ffffffffff;  //ʱ��λ����
        static const __uuint64 UUINT12_MASK = 0xfff;  //����λ����
        static const __uuint64 UUINT10_MASK = 0x3ff; //����λ����

        static const __uuint64 TIME_OFFSET = 22; //ʱ��λƫ����
        static const __uuint64 MACH_OFFSET = 12; //����λƫ����
        static const __uuint64 SEQ_OFFSET = 0; //����λƫ����

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



