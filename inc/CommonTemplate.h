/***********************************************************************
ģ����	: Utils
�ļ���	: CommonTemplate.hpp
����		: lifeng
��������	: 2017/5/25
��ע		: ͨ�ñ���ִ��ģ�壬���ú궨��
**************************************************************************/
#pragma once
#include <boost/shared_ptr.hpp>
namespace Utils
{
	template<class T>
	class BaseExe
	{
		typedef void(T::*Func)();
		typedef bool(T::*BFunc)();
	public:
		void Execute(T *ptr, Func fun)
		{
			boost::bind(fun, ptr)();
		}

		void ExecuteSharePtr(boost::shared_ptr<T> ptr, Func fun)
		{
			boost::bind(fun, ptr.get());
		}

		void ExecuteBool(T *ptr, BFunc fun)
		{
			boost::bind(fun, ptr)();
		}
	};

	#define CHECK_COND_ACT(cond,act1,act2) \
	if (cond)  \
	{		  \
		act1; \
	}		  \
	else	  \
	{		  \
		act2; \
	}

	#define IF_DO(cond,act) \
	if (cond)		\
	{				\
		act;			\
	}
}

