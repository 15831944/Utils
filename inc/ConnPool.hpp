/***********************************************************************
模块名	: Utils
文件名	: Connpool.hpp
作者		: DW
创建日期	: 2016/08/29
备注		: 链接池对象类
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
    * CConnpool是模板类，实例化模板的链接对象只需要实现3个方法 Connect()\DisConnect()\Heartbeat()
    * 为了避免代码书写错误所以建议从IConnection类派生并实现3个纯虚函数
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
    * 默认链接对象生成器
    * 用户可以根据此模板自定义对象生成器
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
            * 在析构时停止定时器并断开所有链接
            */
            m_TimeServ.RemoveAllTimer();
            DisConnect(); 
        }

        //创建所有链接对象并开启定时任务
        int Initialize(int nPoolSize, int nHeartbeatSec, int nReConnectSec)
        {
            int nCount = 0;
            G GetConnection;

            for (int i = 0; i < nPoolSize; i++)
            {
                //创建链接对象
                conn_type pConn(GetConnection());

                //连接成功插入到活动队列， 否则插入到非活动队列
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

        //创建所有链接对象并开启定时任务
        template<class A>
        int Initialize(int nPoolSize, int nHeartbeatSec, int nReConnectSec, const A& a)
        {
            int nCount = 0;

            for (int i = 0; i < nPoolSize; i++)
            {
                //创建链接对象
                conn_type pConn(new T());
                pConn->Initialize(a);

                //连接成功插入到活动队列， 否则插入到非活动队列
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
        //发送心跳（定时任务）
        void Heartbeat()
        {
            //只有活动队列的链接才需要发送心跳
            ConnMap tmpMap;

            /*
            * 复制活动队列到副本
            * 为什么是复制副本而不是swap或者其它方法？
            * 1、活动队列的链接对象需要保留给正常业务使用
            *    如果用swap或者加锁发心跳的模式会导致正常业务拿不到链接对象或长时间等待锁。
            * 2、副本可以有效的记录定时任务触发时活动队列内链接对象的状态
            */

            CopyActive(tmpMap);

            //遍历活动队列链接副本
            for (ConnMapIter Iter = tmpMap.begin(); Iter != tmpMap.end(); ++Iter)
            {
                //查询副本链接对象是否还在活动队列当中
                //如果存在，首先从活动队列移除链接对象并且使用使用副本链接对象发送心跳
                //如果不存在，说明有其它线程将链接对象从活动队列中拿走(无需发送心跳)
                if (PreHeartBeatActive(Iter->first, Iter->second))
                {
                    //发送心跳成功插入到活动队列， 否则插入到非活动队列等待重连
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

        //断线重连（定时任务）
        void ReConnect()
        {
            //只有非活动队列的链接才需要调用重连
            ConnMap tmpMap;

            //使用空的队列与非活动链接队列交换
            SwapInActive(tmpMap);

            for (ConnMapIter Iter = tmpMap.begin(); Iter != tmpMap.end(); ++Iter)
            {
                //重连成功插入到活动队列， 否则插入到非活动队列
                if (Iter->first->Connect())
                    InsertActive(Iter->first, clock_type::local_time());
                else
                    InsertInActive(Iter->first, clock_type::local_time());
            }
        }

        //从连接池中获取可用链接，无可用连接时返回空
        conn_type Pull()
        {
            UniqueLock Locker(m_MutexActive);

            //当前无可用链接
            if (m_MapActive.size() == 0)
                return conn_type();

            //取出链接对象
            ConnMapIter Iter = m_MapActive.begin();
            conn_type pConn = Iter->first;

            //把链接对象从可用队列移除
            m_MapActive.erase(Iter);
            return pConn;
        }

        //归还链接
        void Push(conn_type pConn, bool bActive = true)
        {
            /*
            * 客户端在拿到可用链接处理业务时如果发现链接问题
            * 可以直接将链接归还到非活动队列提高断线重连效率
            * 默认归还到活动队列
            */

            if (bActive)
                InsertActive(pConn, clock_type::local_time());
            else
            {

                //在放入InActive队列之前先主动调用DisConnect
                pConn->DisConnect();
                InsertInActive(pConn, clock_type::local_time());
            }
                
        }

    private:
        void StartTimer(int nHeartbeatSec, int nReConnectSec)
        {
            //启动心跳和断线重连线程与定时器
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

        //断开所有链接(仅析构时调用)
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
