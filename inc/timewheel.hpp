/***********************************************************************
ģ����	: Utils
�ļ���	: timewheel.hpp
����		: DW
��������	: 2016/12/29
��ע		: ʱ���֣������ڸ��ֳ�ʱ���
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
                //�����߳̿���ʱ����
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

        //�������һ�θ���ʱ����һ��weak���������˺�ÿ�ζ�����±��봫��weak������
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
                    //�����߳��жϵ㣬ȷ���߳��ܹ���ȫ�ж�
                    //ÿ���Ӹ���һ����Ƭ
                    //���ѭ������������Ƭ��������ô���Ƚ��뻺��������Ƭ���ᱻ����
                    boost::this_thread::sleep(boost::posix_time::seconds(1));
                    OnTimeUpdate();
                } while (1);
            }
            catch (std::exception&)
            {
                //���̱߳�interrupt֮�󣬽�������쳣
                //����ֻ�������쳣�������κδ���ֻ��Ϊ������ֹͣ�߳�
            }
        }
        void OnTimeUpdate()
        {
            //���ڸ��Ƽ�����������ͷ��ʱ����Ƭ
            //�˶�������ʱ���ᴥ��shared_iterator����,�Ӷ����ÿͻ��Զ����������
            shared_set tmp;

            {
                autolock_t lock(m_Mutex);

                //���ʱ��������, ȡ��������������ͷ��ʱ����Ƭ
                if (m_Wheel.size() == m_Wheel.capacity())
                {
                    shared_set& ref = m_Wheel.front();
                    tmp.swap(ref);
                    m_Wheel.pop_front();
                }

                //�����µ�ʱ����Ƭ��ʱ����β��
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

//ȫ�ֶ���������ڴ洢contxt��shread_ptr
//shared_contxt_set g_shared_contxt_set; 

//�Զ������������
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
// ��¼ʱ���ֵ������������� -- ��Ҫʱ����������ʱ�Ķ���Ӧ���������Ա
// �ڵ�һ�ε���ʱ���ֶ����Push(1)�����������
// ��ȡ����������������֮���ÿ�θ���Ӧ�ô���������(Push(2)����)
//    CContxtWheelWeakIt wit; ��
//};
//
//
//int main(int argc, char* argv[])
//{
//    CContxtWheel Wheel(10);   //��ʱʱ��Ϊ10��
//    Wheel.Run();
//
//    for (int i = 0; i < 5; i++)
//    {
//        shared_contxt sp(new contxt_t(i));
//        g_shared_contxt_set.insert(sp);
//        sp->wit = Wheel.Push(sp); //����Push(1)������ȡ��������������
//    }
//
//    shared_contxt_set::iterator it = g_shared_contxt_set.begin();
//    ++it;
//    for (int i = 0; i < 10000; i++)
//    {
//        Wheel.Push((*it)->wit); //����Push(2)���������±�Ϊ1�Ķ���
//    }
//
//    getchar();
//    return 0;
//}
