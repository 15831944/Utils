/*=========================================================================
描述  ：使用boost库实现的简易计时器。
备注  ：使用方法：创建一个TimerService对象，调用AddTimer添加计时器，调用
		RemoveTimer移除计时器。
日期  ：2016.1 xly
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
	//获得一个定时器对象智能指针
	static boost::shared_ptr<EasyTimer> GetOneTimer(boost::asio::io_service& iosrv)
	{
		return boost::shared_ptr<EasyTimer>(new EasyTimer(iosrv));
	}
	//析构
	~EasyTimer()
	{
		StopTimer();
	}

	//开始计时
	template <typename WaitHandler>
	bool StartTimer(const boost::posix_time::time_duration& expiry_time, const WaitHandler& handler, bool bRepeat = true)
	{
		//真是*了狗了，此处得new一个副本
		boost::shared_ptr<WaitHandler> pHander(new WaitHandler(handler));
		m_repeat = bRepeat;
		m_timedura = expiry_time;
		m_deadtimer.expires_from_now(m_timedura);
		m_deadtimer.async_wait(boost::bind(&EasyTimer::do_handler<WaitHandler>, shared_from_this(), pHander, boost::asio::placeholders::error));
		return true;
	}

	//停止计时
	void StopTimer()
	{
		m_repeat = false;
		m_deadtimer.cancel();
	}

private:
	//私有构造，防止用户创建栈上对象
	EasyTimer(boost::asio::io_service& iosrv) :m_iosrv(iosrv),
		m_repeat(false),
		m_deadtimer(m_iosrv)
	{
	}

	//内部计时器回调
	template <typename WaitHandler>
	void do_handler(const boost::shared_ptr<WaitHandler> phandler, const boost::system::error_code& error)
	{
		if (!error)
		{
			if (m_repeat)
			{
				m_deadtimer.expires_from_now(m_timedura);
				//此处必须携带模板参数，handler必须使用引用
				m_deadtimer.async_wait(boost::bind(&EasyTimer::do_handler<WaitHandler>, shared_from_this(), phandler, boost::asio::placeholders::error));
			}
			(*phandler)();
		}
	}
private:
	boost::asio::io_service&	m_iosrv;
	boost::asio::deadline_timer	m_deadtimer;
	bool						m_repeat;			//是否重复计时
	boost::posix_time::time_duration m_timedura;	//计时间隔
};

/*=========================================================================
类描述：可以管理一组计时器，提供添加和删除功能。
接口  ：
备注  ：
日期  ：2015.11 xly
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

	//添加一个timer，expiry_time：计时器间隔时间，handler：计时器回调函数，bRepeat：是否重复计时，为false表示只计时一次
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

	//移除一个timer
	void RemoveTimer(TIMER_ID timer_id)
	{
		timer_id->StopTimer();
		m_set_timer.erase(timer_id);
	}
	//移除所有timer
	void RemoveAllTimer()
	{
		for (std::set<timer_ptr_t>::iterator it = m_set_timer.begin(); it != m_set_timer.end();it++)
		{
			(*it)->StopTimer();
		}
		m_set_timer.clear();
	}
	//添加一个执行线程
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
//使用示例。每隔1s调用一次TA类中的show函数，10s后，移除计时器。
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
	//此处的sleep仅仅是为了观察计时器效果
	#ifdef WIN32
	Sleep(10000);
	#else
	sleep(10);
	#endif // WIN32
	ts.RemoveTimer(tmid);
	printf("timer is canceled\n");
}
*/


