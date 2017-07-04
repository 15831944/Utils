#ifndef _CGUARD_UTILS_HPP_
#define _CGUARD_UTILS_HPP_
#include <boost/thread/mutex.hpp>  
namespace Utils{
class CGuard : public boost::noncopyable
{
public:
	CGuard()
	{
		taskMutex_.lock();
	}

	
	void wait()
	{
		boost::mutex::scoped_lock sl(taskMutex_);
	}

	virtual void Done()
	{
		taskMutex_.unlock();
	}

	virtual ~CGuard() throw()
	{

	}
private:
	boost::mutex taskMutex_;
};
}
#endif
