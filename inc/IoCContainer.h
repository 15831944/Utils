/***********************************************************************
ģ����	: Utils
�ļ���	: IoCContainer.h
����		: lifeng
��������	: 2017/6/15
��ע		: IOC ��תע������
**************************************************************************/
#pragma once
#include <map>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/call_traits.hpp>

namespace Utils
{

	//�޲ι�����
	template<class DeriveType>
	DeriveType* CreateDeriveType()
	{
		return new DeriveType();
	};

	//����һ�������Ĺ�����
	template<class DeriveType, typename _1P>
	DeriveType* CreateDriveTypeWithParam(typename boost::call_traits<_1P>::param_type value)
	{
		return new DeriveType(value);
	};


	template<typename Key, typename ParentType, typename _1P>
	class CIOCCreaterWithParam
	{
	public:
		CIOCCreaterWithParam(){}
		~CIOCCreaterWithParam() throw(){}

		template<class DeriveType>
		void RegisterType(typename boost::call_traits<Key>::param_type key){
			boost::function<ParentType* (_1P)> function = boost::bind(&CreateDriveTypeWithParam<DeriveType, _1P>, _1);
			Register(key, function);
		}

		ParentType* GetRegisterType(typename boost::call_traits<Key>::param_type key, 
									typename boost::call_traits<_1P>::param_type value)
		{
			typename std::map<Key, boost::function<ParentType*(_1P)> >::const_iterator cIter;
			cIter = m_creator.find(key);
			if (cIter != m_creator.end())
			{
				return (cIter->second)(value);
			}
			return NULL;
		}
	protected:
		template<class DeriveType>
		void Register(typename boost::call_traits<Key>::param_type key, 
					  boost::function<DeriveType*(_1P)> func)
		{
			if (m_creator.find(key) == m_creator.end())
			{
				m_creator.insert(make_pair(key, func));
			}
		}
	private:
		std::map<Key, boost::function<ParentType*(_1P)> > m_creator;
	};

	template<typename Key, typename ParentType>
	class CIOCCreater
	{
	public:
		CIOCCreater(){}
		~CIOCCreater() throw(){}

		template<class DeriveType>
		void RegisterType(typename boost::call_traits<Key>::param_type key){
			boost::function<ParentType* ()> function = boost::bind(&CreateDeriveType<DeriveType>);// []{return new DrivedType();};
			Register(key, function);
		}

		ParentType* GetRegisterType(typename boost::call_traits<Key>::param_type key)
		{
			typename std::map<Key, boost::function<ParentType*()> >::const_iterator cIter;
			cIter = m_creator.find(key);
			if (cIter != m_creator.end())
			{
				return (cIter->second)();
			}
			return NULL;
		}
	protected:
		template<class DeriveType>
		void Register(typename boost::call_traits<Key>::param_type key, boost::function<DeriveType*()> func)
		{
			//typename std::map<Key, boost::function<ParentType*()> >::iterator iter;
			//iter = m_creator.find(key);
			if (m_creator.find(key) == m_creator.end())
			{
				m_creator.insert(make_pair(key, func));
			}
		}
	private:
		std::map<Key, boost::function<ParentType*()> > m_creator;
	};

}

/*
ʹ��ʾ����
˵������������������Ҫ���ڿ��Ʒ�ת��������ʹ�þ���ʹ�ô�����Բο����£��Ľ����ֶο��Բο�
DBUtils��DBPool�еľ�̬ע�� �ֶ�
class A
{
public:
A()
{

}
};

class Ad1 : public A
{
public:
Ad1(const std::string &name)
{
cout << "Ad1" <<name<<endl;
}

Ad1(void)
{
cout << "Ad1 void " << endl;
}

};

class Ad2 : public A
{
public:
Ad2(const std::string &name)
{
cout << "Ad2" <<name<<endl;
}

Ad2(void)
{
cout << "Ad2 void" << endl;
}
};


int _tmain(int argc, _TCHAR* argv[])
{
{
Utils::CIOCCreaterWithParam<TypeEmu, A, const string&> Contaniter;
std::string A1Str = "Ad1";
std::string A2Str = "Ad2";
Contaniter.RegisterType<Ad1>(T_1);
Contaniter.RegisterType<Ad2>(T_2);
A *p1 = Contaniter.GetRegisterType(T_1, A1Str);
A *p2 = Contaniter.GetRegisterType(T_2, A2Str);
}

{
Utils::CIOCCreater<TypeEmu, A> Contaniter0;
const std::string A1Str = "Ad1";
const std::string A2Str = "Ad2";
Contaniter0.RegisterType<Ad1>(T_1);
Contaniter0.RegisterType<Ad2>(T_2);
A *p1 = Contaniter0.GetRegisterType(T_1);
A *p2 = Contaniter0.GetRegisterType(T_2);
}


//TestTimer(TIMER_WHEEL);
//TestTimer(TIMER_HEAP);
//TestTimerWithoutWait(TIMER_HEAP);
return 0;
}



*/

