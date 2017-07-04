#pragma once
#include <boost/interprocess/sync/named_semaphore.hpp>
#include <cstring>
/***********************************************************************
模块名	: Utils
文件名	: Semaphore.hpp
作者		: LCH
创建日期	: 2016/10/31
备注		: 创建有名的信号量，实现跨进程同步。当前进程阻塞住，直到另一个进程post信号量
**************************************************************************/
//创建有名的信号量
namespace Utils
{
	typedef boost::interprocess::named_semaphore NamedSem;
	class CNamedSemaphoreWait
	{
	public:
		CNamedSemaphoreWait(const char *name)
		{
			NamedSem::remove(name);
			pSemNamedWait = new NamedSem(boost::interprocess::create_only_t(), name, 0);
			size_t nlen = strlen(name);
			m_pname = new char[nlen + 1];
			memset(m_pname, 0, nlen + 1);
			memcpy(m_pname, name, nlen);
		}
		//阻塞住
		void Wait()
		{
			pSemNamedWait->wait();
		}
		~CNamedSemaphoreWait()
		{
			if (m_pname != NULL)
			{
				NamedSem::remove(m_pname);
				delete[] m_pname;
				delete   pSemNamedWait;
			}
		}
	private:
		NamedSem *pSemNamedWait;
		char *m_pname;
	};
	//打开有名的信号量
	class CNamedSemaphorePost
	{
	public:
		CNamedSemaphorePost(const char *name) :SemNamedPost(boost::interprocess::open_only_t(), name)
		{
		}
		//发送信号量
		void Post()
		{
			SemNamedPost.post();
		}
		~CNamedSemaphorePost()
		{
		}
	private:
		NamedSem SemNamedPost;
	};
}

//示例
//int main()
//{
//	CNamedSemaphoreWait wait1("AAAA");
//	while (1)
//	{
//		wait1.Wait(); //阻塞住，等待post
//		do something...;
//	}
//	return 1;
//}
//int main()
//{
//	CNamedSemaphorePost post1("AAAA");
//	while (1)
//	{
//		post1.Post();
//		int nn = 123;
//	}
//	return 1;
//}

