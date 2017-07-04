/***********************************************************************
ģ����	: Utils
�ļ���	: MemPool.hpp
����		: DW
��������	: 2016/09/09
��ע		: �ڴ�ض�����
**************************************************************************/
#pragma once

#include <vector>
#include <boost/thread/mutex.hpp>
#include <boost/lockfree/queue.hpp>
using namespace std;

namespace Utils
{
    typedef unsigned long size_type;

    //����ÿ�������ڴ����
    const size_type PER_ALLOC_NUM = 1024;

    //ÿ���ڴ�����С����
    const size_type ALLOC_ROUND_MIN = 8;

    //С���ڴ�����ڵ㶨��
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
            //�����볤�ȶ���Ϊ__ROUND��������
            n = ROUND_UP(n);

            //�����볤�ȴ����������볤��ֱ�ӴӲ���ϵͳ�����ڴ�
            if (n > m_nMax)
                return _AllocateSys(n);

            //�������������ڴ�
            return m_BlockVec[ROUND_INDEX(n)]->_AllocateAux(n);
        }

        void DeAllocate(char* p, size_type n)
        {
            //�����볤�ȶ���Ϊ__ROUND��������
            n = ROUND_UP(n);

            if (n > m_nMax)
                return _DeAllocateSys(p, n);

            //�������������ڴ�
            return m_BlockVec[ROUND_INDEX(n)]->_DeAllocate(p, n);
        }

    private:
        //�����볤��n����Ϊ__ALGIN�ı���
        inline size_type ROUND_UP(size_type n)
        {
            return (n + m_nRound - 1) & ~(m_nRound - 1);
        }

        //ͨ���������n�ҵ���Ӧ�������λ��
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

    typedef CMempoolT<CMemBlock> CMempool;  //�ڴ��
    typedef CMempoolT<CMutexMemBlock> CMutextMempool;//�ڴ��(���̰߳�ȫ)
    typedef CMempoolT<CLockfreeMemBlock> CLockfreeMempool;//�ڴ��(���̰߳�ȫ)
}
