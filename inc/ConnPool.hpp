/***********************************************************************
ģ����	: Utils
�ļ���	: Connpool.hpp
����		: DW
��������	: 2016/08/29
��ע		: ���ӳض�����
**************************************************************************/
#pragma once

#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/date_time.hpp>
#include "EasyTimer.hpp"
using namespace std;

namespace Utils
{
    /*
    * CConnpool��ģ���࣬ʵ����ģ������Ӷ���ֻ��Ҫʵ��3������ Connect()\DisConnect()\Heartbeat()
    * Ϊ�˱��������д�������Խ����IConnection��������ʵ��3�����麯��
    */

    class IConnection
    {
    public:
        virtual ~IConnection(){}
        virtual bool Connect() = 0;
        virtual void DisConnect() = 0;
        virtual bool Heartbeat() = 0;
    };

    /*
    * Ĭ�����Ӷ���������
    * �û����Ը��ݴ�ģ���Զ������������
    */
    template<typename T>
    struct ConnGen
    {
        T* operator()()
        {
            return new(std::nothrow) T;
        }
    };


    /*
    * CConnpool
    */
    template<typename T, typename G = ConnGen<T> >
    class CConnpool 
        : private boost::noncopyable
    {
    public:
        typedef boost::shared_ptr<T> conn_type;
        
    private:
        typedef boost::posix_time::ptime time_type;
        typedef boost::posix_time::microsec_clock clock_type;
        typedef boost::posix_time::time_duration span_type;

        typedef boost::mutex Mutex;
        typedef boost::mutex::scoped_lock UniqueLock;

        typedef map<conn_type, time_type> ConnMap;
        typedef typename ConnMap::iterator ConnMapIter;

    public:
        explicit CConnpool() {}
        ~CConnpool() 
        {
            /*
            * ������ʱֹͣ��ʱ�����Ͽ���������
            */
            m_TimeServ.RemoveAllTimer();
            DisConnect(); 
        }

        //�����������Ӷ��󲢿�����ʱ����
        int Initialize(int nPoolSize, int nHeartbeatSec, int nReConnectSec)
        {
            int nCount = 0;
            G GetConnection;

            for (int i = 0; i < nPoolSize; i++)
            {
                //�������Ӷ���
                conn_type pConn(GetConnection());

                //���ӳɹ����뵽����У� ������뵽�ǻ����
                if (pConn->Connect())
                {
                    m_MapActive.insert(make_pair(pConn, clock_type::local_time()));
                    nCount++;
                }                
                else
                    m_MapInActive.insert(make_pair(pConn, clock_type::local_time()));
            }
           
            StartTimer(nHeartbeatSec, nReConnectSec);
            return nCount;
        }

        //�����������Ӷ��󲢿�����ʱ����
        template<class A>
        int Initialize(int nPoolSize, int nHeartbeatSec, int nReConnectSec, const A& a)
        {
            int nCount = 0;

            for (int i = 0; i < nPoolSize; i++)
            {
                //�������Ӷ���
                conn_type pConn(new T());
                pConn->Initialize(a);

                //���ӳɹ����뵽����У� ������뵽�ǻ����
                if (pConn->Connect())
                {
                    m_MapActive.insert(make_pair(pConn, clock_type::local_time()));
                    nCount++;
                }
                else
                    m_MapInActive.insert(make_pair(pConn, clock_type::local_time()));
            }

            StartTimer(nHeartbeatSec, nReConnectSec);
            return nCount;
        }
        //������������ʱ����
        void Heartbeat()
        {
            //ֻ�л���е����Ӳ���Ҫ��������
            ConnMap tmpMap;

            /*
            * ���ƻ���е�����
            * Ϊʲô�Ǹ��Ƹ���������swap��������������
            * 1������е����Ӷ�����Ҫ����������ҵ��ʹ��
            *    �����swap���߼�����������ģʽ�ᵼ������ҵ���ò������Ӷ����ʱ��ȴ�����
            * 2������������Ч�ļ�¼��ʱ���񴥷�ʱ����������Ӷ����״̬
            */

            CopyActive(tmpMap);

            //������������Ӹ���
            for (ConnMapIter Iter = tmpMap.begin(); Iter != tmpMap.end(); ++Iter)
            {
                //��ѯ�������Ӷ����Ƿ��ڻ���е���
                //������ڣ����ȴӻ�����Ƴ����Ӷ�����ʹ��ʹ�ø������Ӷ���������
                //��������ڣ�˵���������߳̽����Ӷ���ӻ����������(���跢������)
                if (PreHeartBeatActive(Iter->first, Iter->second))
                {
                    //���������ɹ����뵽����У� ������뵽�ǻ���еȴ�����
                    if (Iter->first->Heartbeat())
                    {
                        InsertActive(Iter->first, clock_type::local_time());
                    }                  
                    else
                    {
                        Iter->first->DisConnect();
                        InsertInActive(Iter->first, clock_type::local_time());
                    }
                        
                }
            }
        }

        //������������ʱ����
        void ReConnect()
        {
            //ֻ�зǻ���е����Ӳ���Ҫ��������
            ConnMap tmpMap;

            //ʹ�ÿյĶ�����ǻ���Ӷ��н���
            SwapInActive(tmpMap);

            for (ConnMapIter Iter = tmpMap.begin(); Iter != tmpMap.end(); ++Iter)
            {
                //�����ɹ����뵽����У� ������뵽�ǻ����
                if (Iter->first->Connect())
                    InsertActive(Iter->first, clock_type::local_time());
                else
                    InsertInActive(Iter->first, clock_type::local_time());
            }
        }

        //�����ӳ��л�ȡ�������ӣ��޿�������ʱ���ؿ�
        conn_type Pull()
        {
            UniqueLock Locker(m_MutexActive);

            //��ǰ�޿�������
            if (m_MapActive.size() == 0)
                return conn_type();

            //ȡ�����Ӷ���
            ConnMapIter Iter = m_MapActive.begin();
            conn_type pConn = Iter->first;

            //�����Ӷ���ӿ��ö����Ƴ�
            m_MapActive.erase(Iter);
            return pConn;
        }

        //�黹����
        void Push(conn_type pConn, bool bActive = true)
        {
            /*
            * �ͻ������õ��������Ӵ���ҵ��ʱ���������������
            * ����ֱ�ӽ����ӹ黹���ǻ������߶�������Ч��
            * Ĭ�Ϲ黹�������
            */

            if (bActive)
                InsertActive(pConn, clock_type::local_time());
            else
            {

                //�ڷ���InActive����֮ǰ����������DisConnect
                pConn->DisConnect();
                InsertInActive(pConn, clock_type::local_time());
            }
                
        }

    private:
        void StartTimer(int nHeartbeatSec, int nReConnectSec)
        {
            //���������Ͷ��������߳��붨ʱ��
            m_TimeServ.AddTimer(\
                boost::posix_time::seconds(nHeartbeatSec), \
                boost::bind(&CConnpool::Heartbeat, this));

            m_TimeServ.AddTimer(\
                boost::posix_time::seconds(nReConnectSec), \
                boost::bind(&CConnpool::ReConnect, this));
        }

        bool PreHeartBeatActive(conn_type pConn, time_type ActiveTime)
        {
            UniqueLock Locker(m_MutexActive);
            ConnMapIter Iter = m_MapActive.find(pConn);

            if (Iter != m_MapActive.end() && Iter->second <= ActiveTime)
            {
                m_MapActive.erase(Iter);
                return true;
            }

            return false;
        }

        void CopyActive(ConnMap& tmpMap)
        {
            UniqueLock Locker(m_MutexActive);
            tmpMap = m_MapActive;
        }

        void InsertActive(conn_type pConn, time_type ActiveTime)
        {
            UniqueLock Locker(m_MutexActive);
            m_MapActive.insert(make_pair(pConn, ActiveTime));
        }

        void SwapInActive(ConnMap& tmpMap)
        {
            UniqueLock Locker(m_MutexInActive);
            m_MapInActive.swap(tmpMap);
        }

        void InsertInActive(conn_type pConn, time_type ActiveTime)
        {
            UniqueLock Locker(m_MutexInActive);
            m_MapInActive.insert(make_pair(pConn, ActiveTime));
        }

        //�Ͽ���������(������ʱ����)
        void DisConnect()
        {
            UniqueLock Locker(m_MutexActive);
            ConnMapIter Iter = m_MapActive.begin();
            for (; Iter != m_MapActive.end(); ++Iter)
                Iter->first->DisConnect();
        }

    private:
        ConnMap m_MapActive;
        Mutex   m_MutexActive;

        ConnMap m_MapInActive;
        Mutex   m_MutexInActive;

        TimerService m_TimeServ;
    };
}
