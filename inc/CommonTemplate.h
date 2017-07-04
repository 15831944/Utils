/***********************************************************************
模块名	: Utils
文件名	: CommonTemplate.hpp
作者		: lifeng
创建日期	: 2017/5/25
备注		: 通用遍历执行模板，常用宏定义
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

