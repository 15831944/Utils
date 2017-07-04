#pragma once
#include <boost/interprocess/sync/named_semaphore.hpp>
#include <cstring>
/***********************************************************************
ģ����	: Utils
�ļ���	: Semaphore.hpp
����		: LCH
��������	: 2016/10/31
��ע		: �����������ź�����ʵ�ֿ����ͬ������ǰ��������ס��ֱ����һ������post�ź���
**************************************************************************/
//�����������ź���
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
		//����ס
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
	//���������ź���
	class CNamedSemaphorePost
	{
	public:
		CNamedSemaphorePost(const char *name) :SemNamedPost(boost::interprocess::open_only_t(), name)
		{
		}
		//�����ź���
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

//ʾ��
//int main()
//{
//	CNamedSemaphoreWait wait1("AAAA");
//	while (1)
//	{
//		wait1.Wait(); //����ס���ȴ�post
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

