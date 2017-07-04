#pragma once
#include <boost/noncopyable.hpp>

template<typename Ty>
class CSingleton : public boost::noncopyable
{
public:
	virtual ~CSingleton()
	{
		
	}
	
	static Ty& GetInstance()
	{
		static Ty _Unique;
		return _Unique;
	}
};

//example
/*
class CTreeMgr : public CSingleton<CTreeMgr>
{
public:
	bool Initialize()
	{
		return true;
	}
};

int main()
{
	CTreeMgr& Mgr = CTreeMgr::GetInstance();
	Mgr.Initialize();
	return -1;
}
*/

