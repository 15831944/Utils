/***********************************************************************
模块名	: Utils
文件名	: MemPool.hpp
作者		: DW
创建日期	: 2016/09/09
备注		: 内存池对象类
**************************************************************************/
#pragma once

#include <vector>
#include <boost/thread/mutex.hpp>
#include <boost/lockfree/queue.hpp>
using namespace std;

namespace Utils
{
    typedef unsigned long size_type;

    //定义每次申请内存块数
    const size_type PER_ALLOC_NUM = 1024;

    //每个内存块间最小长度
    const size_type ALLOC_ROUND_MIN = 8;

    //小块内存链表节点定义
    typedef struct MemObjLink
    {
        MemObjLink* pNext;
    }MEMOBJLINK, *PMEMOBJLINK;

    inline char* _AllocateSys(size_type n)
    {
        return new char[n];
    }

    inline void _DeAllocateSys(char* p, size_type n)
    {
        delete[] p;
    }

    class CMemBlock
    {
    public:
        CMemBlock(size_type n){ m_MemObjLink.pNext = 0; }
        virtual ~CMemBlock(){ _Clear(); }

        virtual char* _AllocateAux(size_type n)
        {
            MemObjLink* pResult = m_MemObjLink.pNext;
            if (!pResult)
                return _AllocateRaw(n);

            m_MemObjLink.pNext = pResult->pNext;
            return (char*)pResult;
        }

        virtual void _DeAllocate(char* p, size_type n)
        {
            PMEMOBJLINK pNextLink = (PMEMOBJLINK)p;
            pNextLink->pNext = m_MemObjLink.pNext;
            m_MemObjLink.pNext = pNextLink;
        }

        virtual void _Clear()
        {
            for (size_t i = 0; i < m_AllocVec.size(); i++)
            {
                delete m_AllocVec[i];
            }
        }

    private:
        char* _AllocateRaw(size_type n)
        {
            char* ptr = _AllocateSys(n * PER_ALLOC_NUM);

            if (!ptr)
                return 0;

            m_AllocVec.push_back(ptr);
            PMEMOBJLINK pNextLink;
            PMEMOBJLINK pCurrent = &m_MemObjLink;
            for (size_type i = 1; i < PER_ALLOC_NUM; ++i)
            {
                pNextLink = (PMEMOBJLINK)(ptr + i * n);
                pCurrent->pNext = pNextLink;
                pCurrent = pNextLink;
            }

            pCurrent->pNext = 0;
            return ptr;
        }

        CMemBlock(const CMemBlock& Ref);
        CMemBlock& operator =(const CMemBlock& Ref);

    private:
        vector<char*> m_AllocVec;
        MemObjLink m_MemObjLink;
    };

    class CMutexMemBlock : public CMemBlock
    {
        typedef boost::mutex MutexImpl;
        typedef boost::mutex::scoped_lock AutoLockImpl;

    public:
        CMutexMemBlock(size_type n) : CMemBlock(n){}
        virtual ~CMutexMemBlock(){}

        virtual char* _AllocateAux(size_type n)
        {
            AutoLockImpl Lock(m_Mutex);
            return CMemBlock::_AllocateAux(n);
        }

        virtual void _DeAllocate(char* p, size_type n)
        {
            AutoLockImpl Lock(m_Mutex);
            return CMemBlock::_DeAllocate(p, n);
        }

    private:
        MutexImpl m_Mutex;
    };

    class CLockfreeMemBlock
    {
        typedef boost::lockfree::queue<char*> LockfreeMemObjLink;

    public:
        CLockfreeMemBlock(size_type n) 
            : m_MemObjLink(0)
        {
        }
        virtual ~CLockfreeMemBlock(){ _Clear(); }

        virtual char* _AllocateAux(size_type n)
        {
            if (m_MemObjLink.empty())
                return _AllocateRaw(n);

            char* p = 0;
            while (!m_MemObjLink.pop(p));
            return p;
        }

        virtual void _DeAllocate(char* p, size_type n)
        {
            while (!m_MemObjLink.push(p));
        }

        virtual void _Clear()
        {
            for (size_t i = 0; i < m_AllocVec.size(); i++)
            {
                delete m_AllocVec[i];
            }
        }

    private:
        char* _AllocateRaw(size_type n)
        {
            char* ptr = _AllocateSys(n * PER_ALLOC_NUM);

            if (!ptr)
                return 0;

            m_AllocVec.push_back(ptr);
            for (size_type i = 1; i < PER_ALLOC_NUM; ++i)
            {
                while (!m_MemObjLink.push(ptr + i));
            }

            return ptr;
        }

        CLockfreeMemBlock(const CLockfreeMemBlock& Ref);
        CLockfreeMemBlock& operator =(const CLockfreeMemBlock& Ref);

    private:
        vector<char*> m_AllocVec;
        LockfreeMemObjLink m_MemObjLink;
    };

    template<typename T>
    class CMempoolT
    {
    public:
        CMempoolT(size_type nRound, size_type nMax)
        {
            m_nRound = nRound > ALLOC_ROUND_MIN ? nRound : ALLOC_ROUND_MIN;
            m_nMax = nMax == 0 ? nRound : ROUND_UP(nMax);

            for (size_type i = 0; i < m_nMax / m_nRound; i++)
            {
                m_BlockVec.push_back(new T((i + 1) * m_nRound));
            }
        }

        ~CMempoolT()
        {
            for (size_t i = 0; i < m_BlockVec.size(); i++)
            {
                delete m_BlockVec[i];
            }
        }

        char* Allocate(size_type n)
        {
            //将申请长度对齐为__ROUND的整数倍
            n = ROUND_UP(n);

            //当申请长度大于最大可申请长度直接从操作系统申请内存
            if (n > m_nMax)
                return _AllocateSys(n);

            //从索引表申请内存
            return m_BlockVec[ROUND_INDEX(n)]->_AllocateAux(n);
        }

        void DeAllocate(char* p, size_type n)
        {
            //将申请长度对齐为__ROUND的整数倍
            n = ROUND_UP(n);

            if (n > m_nMax)
                return _DeAllocateSys(p, n);

            //从索引表申请内存
            return m_BlockVec[ROUND_INDEX(n)]->_DeAllocate(p, n);
        }

    private:
        //将输入长度n对齐为__ALGIN的倍数
        inline size_type ROUND_UP(size_type n)
        {
            return (n + m_nRound - 1) & ~(m_nRound - 1);
        }

        //通过输入参数n找到对应索引表的位置
        inline size_type ROUND_INDEX(size_type n)
        {
            return n / m_nRound - 1;
        }

        CMempoolT(const CMempoolT& Ref);
        CMempoolT& operator =(const CMempoolT& Ref);

    private:
        size_t m_nMax;
        size_t m_nRound;
        vector<T*> m_BlockVec;
    };

    typedef CMempoolT<CMemBlock> CMempool;  //内存池
    typedef CMempoolT<CMutexMemBlock> CMutextMempool;//内存池(多线程安全)
    typedef CMempoolT<CLockfreeMemBlock> CLockfreeMempool;//内存池(多线程安全)
}
