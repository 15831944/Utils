/*=========================================================================
����  ��ʹ��boost��ʵ�ֵļ��׼�ʱ����
��ע  ��ʹ�÷���������һ��TimerService���󣬵���AddTimer��Ӽ�ʱ��������
		RemoveTimer�Ƴ���ʱ����
����  ��2016.1 xly
=========================================================================*/
#pragma once

#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <vector>
#include <set>
class EasyTimer: public boost::enable_shared_from_this<EasyTimer>
{
public:
	//���һ����ʱ����������ָ��
	static boost::shared_ptr<EasyTimer> GetOneTimer(boost::asio::io_service& iosrv)
	{
		return boost::shared_ptr<EasyTimer>(new EasyTimer(iosrv));
	}
	//����
	~EasyTimer()
	{
		StopTimer();
	}

	//��ʼ��ʱ
	template <typename WaitHandler>
	bool StartTimer(const boost::posix_time::time_duration& expiry_time, const WaitHandler& handler, bool bRepeat = true)
	{
		//����*�˹��ˣ��˴���newһ������
		boost::shared_ptr<WaitHandler> pHander(new WaitHandler(handler));
		m_repeat = bRepeat;
		m_timedura = expiry_time;
		m_deadtimer.expires_from_now(m_timedura);
		m_deadtimer.async_wait(boost::bind(&EasyTimer::do_handler<WaitHandler>, shared_from_this(), pHander, boost::asio::placeholders::error));
		return true;
	}

	//ֹͣ��ʱ
	void StopTimer()
	{
		m_repeat = false;
		m_deadtimer.cancel();
	}

private:
	//˽�й��죬��ֹ�û�����ջ�϶���
	EasyTimer(boost::asio::io_service& iosrv) :m_iosrv(iosrv),
		m_repeat(false),
		m_deadtimer(m_iosrv)
	{
	}

	//�ڲ���ʱ���ص�
	template <typename WaitHandler>
	void do_handler(const boost::shared_ptr<WaitHandler> phandler, const boost::system::error_code& error)
	{
		if (!error)
		{
			if (m_repeat)
			{
				m_deadtimer.expires_from_now(m_timedura);
				//�˴�����Я��ģ�������handler����ʹ������
				m_deadtimer.async_wait(boost::bind(&EasyTimer::do_handler<WaitHandler>, shared_from_this(), phandler, boost::asio::placeholders::error));
			}
			(*phandler)();
		}
	}
private:
	boost::asio::io_service&	m_iosrv;
	boost::asio::deadline_timer	m_deadtimer;
	bool						m_repeat;			//�Ƿ��ظ���ʱ
	boost::posix_time::time_duration m_timedura;	//��ʱ���
};

/*=========================================================================
�����������Թ���һ���ʱ�����ṩ��Ӻ�ɾ�����ܡ�
�ӿ�  ��
��ע  ��
����  ��2015.11 xly
=========================================================================*/
class TimerService
{
	typedef boost::shared_ptr<boost::thread>	thread_ptr_t;
	typedef boost::shared_ptr<EasyTimer>		timer_ptr_t;
public:
	typedef boost::shared_ptr<EasyTimer>		TIMER_ID;
public:
	TimerService(bool bExplicitStart = false) :m_io_work(m_iosrv)
	{
		if (false == bExplicitStart)
		{
			AddThread(1);
		}
	}
	~TimerService()
	{
		m_set_timer.clear();
		m_iosrv.stop();
		for (std::vector<thread_ptr_t>::iterator it = m_vec_thd.begin(); it != m_vec_thd.end();it++)
		{
			(*it)->join();
		}
		m_vec_thd.clear();
	}

	size_t Start_sync()
	{
		return m_iosrv.run();
	}

    size_t StartSync()
    {
        return m_iosrv.run();
    }

	boost::asio::io_service& GetIoService()
	{
		return m_iosrv;
	}

	//���һ��timer��expiry_time����ʱ�����ʱ�䣬handler����ʱ���ص�������bRepeat���Ƿ��ظ���ʱ��Ϊfalse��ʾֻ��ʱһ��
	template <typename WaitHandler>
	TIMER_ID AddTimer(const boost::posix_time::time_duration& expiry_time, const WaitHandler& handler, bool bRepeat = true)
	{
		timer_ptr_t timerptr = EasyTimer::GetOneTimer(m_iosrv);
		if (timerptr)
		{
			timerptr->StartTimer<WaitHandler>(expiry_time, handler, bRepeat);
			m_set_timer.insert(timerptr);
		}
		return timerptr;
	}

	//�Ƴ�һ��timer
	void RemoveTimer(TIMER_ID timer_id)
	{
		timer_id->StopTimer();
		m_set_timer.erase(timer_id);
	}
	//�Ƴ�����timer
	void RemoveAllTimer()
	{
		for (std::set<timer_ptr_t>::iterator it = m_set_timer.begin(); it != m_set_timer.end();it++)
		{
			(*it)->StopTimer();
		}
		m_set_timer.clear();
	}
	//���һ��ִ���߳�
	bool AddThread(int num)
	{
		for (int cnt = 0; cnt < num;++cnt)
		{
			thread_ptr_t ptr(new boost::thread(boost::bind(&boost::asio::io_service::run, &m_iosrv)));
			if (!ptr)
			{
				return false;
			}
			m_vec_thd.push_back(ptr);
		}
		return true;
	}

private:
	boost::asio::io_service				m_iosrv;
	boost::asio::io_service::work		m_io_work;
	std::vector<thread_ptr_t>			m_vec_thd;
	std::set<timer_ptr_t>				m_set_timer;
};

/*
//ʹ��ʾ����ÿ��1s����һ��TA���е�show������10s���Ƴ���ʱ����
class TA
{
public:
	void show(int i, const string& str)
	{
		printf("=============%d,%s\n");
	}
};
void Test()
{
	TA ta;
	TimerService ts;
	TimerService::TIMER_ID tmid = ts.AddTimer(boost::posix_time::seconds(1), boost::bind(&TA::show, &ta, 333, "hehehe\n"));
	//�˴���sleep������Ϊ�˹۲��ʱ��Ч��
	#ifdef WIN32
	Sleep(10000);
	#else
	sleep(10);
	#endif // WIN32
	ts.RemoveTimer(tmid);
	printf("timer is canceled\n");
}
*/


