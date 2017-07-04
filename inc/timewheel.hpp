/***********************************************************************
模块名	: Utils
文件名	: timewheel.hpp
作者		: DW
创建日期	: 2016/12/29
备注		: 时间轮，适用于各种超时检测
**************************************************************************/

#pragma once

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <boost/circular_buffer.hpp>

namespace Utils
{
    template<typename T>
    struct CTimeWheelDtor
    {
        typedef boost::weak_ptr<T> weak_type;
        void operator()(const weak_type& sp){}
    };

    template<typename T, typename Dtor>
    struct CTimeWheelIterator
    {
        typedef boost::weak_ptr<T> weak_type;
        typedef boost::shared_ptr<T> shared_type;

        CTimeWheelIterator(const shared_type& sp)
            : m_pWeak(sp)
        {

        }

        ~CTimeWheelIterator()
        {
            Dtor MyDtor;
            MyDtor(m_pWeak);
        }

        weak_type m_pWeak;
    };

    template<typename T, typename D = CTimeWheelDtor<T> >
    class CTimeWheel
    {
    public:
        typedef CTimeWheelIterator<T, D> iterator;
        typedef boost::weak_ptr<iterator> weak_iterator;
        typedef boost::shared_ptr<iterator> shared_iterator;
        typedef typename iterator::weak_type weak_type;
        typedef typename iterator::shared_type shared_type;

    private:
        typedef boost::unordered_set<shared_iterator> shared_set;
        typedef boost::circular_buffer<shared_set> shared_wheel;

        typedef boost::thread thread_t;
        typedef boost::shared_ptr<thread_t> shared_thread;

        typedef boost::mutex mutex_t;
        typedef boost::mutex::scoped_lock autolock_t;

    public:
        CTimeWheel(unsigned int nKeepSec)
            : m_Wheel(nKeepSec)
        {
            OnTimeUpdate();
        }

        ~CTimeWheel()
        {
            Stop();
        }

        bool Run()
        {
            try
            {
                //启动线程开启时间轮
                m_pThread.reset(new thread_t(
                    boost::bind(&CTimeWheel::Loop, this)));
                return true;
            }
            catch (std::exception&)
            {
                return false;
            }         
        }

        void Stop()
        {
            if (m_pThread)
            {
                m_pThread->interrupt();
                m_pThread->join();
                m_pThread.reset();
            }

            clear();
        }

        //当对象第一次更新时返回一个weak迭代器，此后每次对象更新必须传入weak迭代器
        weak_iterator Push(const shared_type& sp)
        {
            shared_iterator it(new iterator(sp));
            this->push_back(it);
            return weak_iterator(it);            
        }

        void Push(const weak_iterator& wit)
        {
            shared_iterator it = wit.lock();
            if (it) this->push_back(it);              
        }
        
    private:
        void clear()
        {
            autolock_t lock(m_Mutex);
            m_Wheel.clear();
        }

        void push_back(shared_iterator& it)
        {
            autolock_t lock(m_Mutex);
            m_Wheel.back().insert(it);
        }

        void Loop()
        {
            try
            {
                do
                {
                    //设置线程中断点，确保线程能够安全中断
                    //每秒钟更新一次轮片
                    //如果循环缓冲区内轮片已满，那么最先进入缓冲区的轮片将会被析构
                    boost::this_thread::sleep(boost::posix_time::seconds(1));
                    OnTimeUpdate();
                } while (1);
            }
            catch (std::exception&)
            {
                //当线程被interrupt之后，将会产生异常
                //这里只捕获了异常但不做任何处理，只是为了正常停止线程
            }
        }
        void OnTimeUpdate()
        {
            //用于复制即将被弹出的头部时间轮片
            //此对象析构时将会触发shared_iterator析构,从而调用客户自定义的析构器
            shared_set tmp;

            {
                autolock_t lock(m_Mutex);

                //如果时间轮已满, 取出即将被弹出的头部时间轮片
                if (m_Wheel.size() == m_Wheel.capacity())
                {
                    shared_set& ref = m_Wheel.front();
                    tmp.swap(ref);
                    m_Wheel.pop_front();
                }

                //加入新的时间轮片到时间轮尾部
                m_Wheel.push_back(shared_set());
            }        
        }

    private:
        mutex_t m_Mutex;
        shared_wheel m_Wheel;
        shared_thread m_pThread;
    };
}

//example
//class contxt_t;
//typedef boost::weak_ptr<contxt_t> weak_contxt;
//typedef boost::shared_ptr<contxt_t> shared_contxt;
//
//typedef set<shared_contxt> shared_contxt_set;

//全局对象管理用于存储contxt的shread_ptr
//shared_contxt_set g_shared_contxt_set; 

//自定义对象析构器
//struct contxt_dtor
//{
//    void operator()(const weak_contxt& wp)
//    {
//        shared_contxt sp = wp.lock();
//        if (sp)
//            g_shared_contxt_set.erase(sp);
//    }
//};
//
//typedef Utils::CTimeWheel<contxt_t, contxt_dtor> CContxtWheel;
//typedef CContxtWheel::weak_iterator CContxtWheelWeakIt;
//
//class contxt_t
//{
//public:
//    contxt_t(int i) : m(i)
//    {
//        cout << __FUNCTION__ << m << endl;
//    }
//
//    ~contxt_t()
//    {
//        cout << __FUNCTION__ << m << endl;
//    }
//
//    int m;
//
// 记录时间轮迭代器的弱引用 -- 需要时间轮来管理超时的对象都应该有这个成员
// 在第一次调用时间轮对象的Push(1)方法方法获得
// 获取到迭代器的弱引用之后的每次更新应该传入弱引用(Push(2)方法)
//    CContxtWheelWeakIt wit; ，
//};
//
//
//int main(int argc, char* argv[])
//{
//    CContxtWheel Wheel(10);   //超时时间为10秒
//    Wheel.Run();
//
//    for (int i = 0; i < 5; i++)
//    {
//        shared_contxt sp(new contxt_t(i));
//        g_shared_contxt_set.insert(sp);
//        sp->wit = Wheel.Push(sp); //调用Push(1)方法获取迭代器的弱引用
//    }
//
//    shared_contxt_set::iterator it = g_shared_contxt_set.begin();
//    ++it;
//    for (int i = 0; i < 10000; i++)
//    {
//        Wheel.Push((*it)->wit); //调用Push(2)方法更新下标为1的对象
//    }
//
//    getchar();
//    return 0;
//}
