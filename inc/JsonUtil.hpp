#pragma once

#include <string>
#include <json/json.h>
#include <boost/bind.hpp>
#include <StringFunc.hpp>
using namespace std;

namespace JsonUtil
{
    typedef Json::Value::UInt size_type;

	/*
	* 函数名称： CopyAry(回调)
	* 使用说明： 数组复制
	* 输入参数<nIndex>： 源对象的下标
	* 输入参数<jSrc>： 源对象
	* 输出参数<nNewIndex>： 拷贝对象的下标
	* 输出参数<jNew>： 拷贝对象的Array
	* 返回值: true-已创建并插入数组成员到数组jNew， false-未创建数组成员到数组jNew
	*/
	typedef bool(*CopyAry)(size_type nIndex, const Json::Value& jSrc, size_type& nNewIndex, Json::Value& jNew);

	/*
	* 函数名称： CopyObj(回调)
	* 使用说明： 对象复制
	* 输入参数<srcKey>： 源对象的KEY
	* 输入参数<jSrc>： 源对象
	* 输出参数<newKey>： 拷贝对象的KEY
	* 输出参数<jNew>： 拷贝对象的Parent
	* 返回值: true-已创建并插入数组成员到数组jNew， false-未创建数组成员到数组jNew
	*/
	typedef bool(*CopyObj)(const string& srcKey, const Json::Value& jSrc, string& newKey, Json::Value& jNew);

	/*
	* 函数名称： CopyEle(回调)
	* 使用说明： 元素复制
	* 输入参数<srcKey>： jSrc源对象的KEY
	* 输入参数<srcValue>： 源对象的值(所有值都被转换成string)
	* 输出参数<newKey>： 拷贝对象的KEY
	* 输出参数<jNew>： 拷贝对象的Parent
	* 返回值: true-已创建并插入数组成员到数组jNew， false-未创建数组成员到数组jNew
	*/
	typedef bool(*CopyEle)(const string& srcKey, const string& srcValue, string& newKey, Json::Value& jNew);

	//默认数组复制函数
	static inline 
	bool CopyArrayDefault(size_type nIndex, const Json::Value& jSrc, size_type& nNewIndex, Json::Value& jNew)
	{
		nNewIndex = nIndex;
		//应该避免将jSrc对象直接拷贝到jNew下面，否则对象会重复
		jNew[nNewIndex] = Json::Value();
		return true;
	}
	//默认对象复制函数
	static inline 
	bool CopyObjectDefault(const string& srcKey, const Json::Value& jSrc, string& newKey, Json::Value& jNew)
	{
		newKey = srcKey;
		//应该避免将jSrc对象直接拷贝到jNew下面，否则对象会重复
		jNew[newKey] = Json::Value();
		return true;
	}
	//默认元素复制函数
	static inline  
	bool CopyElementDefault(const string& srcKey, const string& srcValue, string& newKey, Json::Value& jNew)
	{
		newKey = srcKey;
		jNew[newKey] = srcValue;
		return true;
	}
    //由于和JsonArrayCopy互相调用，高版本的GCC编译失败，必须前置声明
	template<typename E, typename O, typename A>
	bool JsonObjectCopy(const Json::Value& jSrc, Json::Value& jNew, const E& CopyE, const O& CopyO, const A& CopyA);
	
	template<typename E, typename O, typename A>
	bool JsonArrayCopy(const Json::Value& jArray, Json::Value& jNew, const E& CopyE, const O& CopyO, const A& CopyA)
	{
		//循环遍历JSON数组
		for (size_type i = 0; i < jArray.size(); ++i)
		{
			size_type nNewIndex = 0;
			const Json::Value& jValue = jArray[i];
			if (CopyA(i, jValue, nNewIndex, jNew))
			{
				//获取回调当中创建的数组成员对象
				Json::Value& jNewObject = jNew[nNewIndex];
				//如果返回的对象type不是null或者object，程序会抛出异常
				if (Json::nullValue == jNewObject.type() || Json::objectValue == jNewObject.type())
				{
					//对象递归
                    JsonObjectCopy<E, O, A>(jValue, jNewObject, CopyE, CopyO, CopyA);
				}
			}
		}

		return true;
	}

	/*
	* 函数名称： JsonObjectCopy
	* 使用说明： Json对象拷贝
	* 输入参数<jSrc>： JSON输入对象
	* 输入参数<jNew>： JSON输出对象
	* 输出参数<CopyE>： 元素复制回调函数，函数原型：CopyEle， 回调对象必须是boost::bind()创建
	* 输出参数<CopyO>： 对象复制回调函数，函数原型：CopyObj， 回调对象必须是boost::bind()创建
	* 输出参数<CopyA>： 数组复制回调函数，函数原型：CopyAry， 回调对象必须是boost::bind()创建
	* 返回值: true-成功 false-失败
	*/
	template<typename E, typename O, typename A>
	bool JsonObjectCopy(const Json::Value& jSrc, Json::Value& jNew, const E& CopyE, const O& CopyO, const A& CopyA)
	{
		//循环遍历JSON对象
		Json::Value::Members jMembers = jSrc.getMemberNames();
		Json::Value::Members::const_iterator iter = jMembers.begin();

		for (; iter != jMembers.end(); iter++)
		{
			const Json::Value& jValue = jSrc[*iter];

			switch (jValue.type())
			{
			case Json::arrayValue:
			{
				//拷贝数组对象
				string strNewKey;
				//如果回调当中拷贝了当前数组对象，则继续遍历该数组，否则跳过当前对象的所有子节点
				if (CopyO(*iter, jValue, strNewKey, jNew))
				{	
					Json::Value& jNewObject = jNew[strNewKey];
					//如果返回的对象type不是null或者array，程序会抛出异常
					if (Json::nullValue == jNewObject.type() || Json::arrayValue == jNewObject.type())
					{
						//数组对象递归
                        JsonArrayCopy<E, O, A>(jValue, jNewObject, CopyE, CopyO, CopyA);
					}		
				}
				break;
			}
			case Json::objectValue:
			{
				//拷贝数组对象
				string strNewKey;
				//如果回调当中拷贝了当前对象，则继续遍历该对象，否则跳过当前对象的所有子节点
				if (CopyO(*iter, jValue, strNewKey, jNew))
				{
					Json::Value& jNewObject = jNew[strNewKey];
					//如果返回的对象type不是null或者object，程序会抛出异常
					if (Json::nullValue == jNewObject.type() || Json::objectValue == jNewObject.type())
					{
						//对象递归
                        JsonObjectCopy<E, O, A>(jValue, jNewObject, CopyE, CopyO, CopyA);
					}
				}
				break;
			}
			//将所有基本元素转换成string然后进行拷贝
			case Json::intValue:
			{
				string strNewKey;
				string strValue = StringUtil::lexical_cast<string>(jValue.asInt(), "0");
				CopyE(*iter, strValue, strNewKey, jNew);
				break;
			}
			case Json::uintValue:
			{
				string strNewKey;
                string strValue = StringUtil::lexical_cast<string>(jValue.asUInt(), "0");
				CopyE(*iter, strValue, strNewKey, jNew);
				break;
			}
			case Json::realValue:
			{
				string strNewKey;
                string strValue = StringUtil::lexical_cast<string>(jValue.asDouble(), "0");
				CopyE(*iter, strValue, strNewKey, jNew);
				break;
			}
			case Json::booleanValue:
			{
				string strNewKey;
                string strValue = StringUtil::lexical_cast<string>(jValue.asBool(), "bool");
				CopyE(*iter, strValue, strNewKey, jNew);
				break;
			}
			case Json::nullValue:
			{
				string strNewKey;
				CopyE(*iter, "", strNewKey, jNew);
				break;
			}
			case Json::stringValue:
			{
				string strNewKey;
				string stringJV = jValue.asString();
				CopyE(*iter, stringJV, strNewKey, jNew);
				break;
			}
			default:
				break;
			}
		}

		return true;
	}

	template<typename E, typename O>
	bool JsonObjectCopy(const Json::Value& jSrc, Json::Value& jNew, const E& CopyE, const O& CopyO)
	{
		return JsonObjectCopy(jSrc, jNew, CopyE, CopyO,
			boost::bind(&JsonUtil::CopyArrayDefault, _1, _2, _3, _4)
			);
	}

	template<typename E>
	bool JsonObjectCopy(const Json::Value& jSrc, Json::Value& jNew, const E& CopyE)
	{
		return JsonObjectCopy(jSrc, jNew, CopyE,
			boost::bind(&JsonUtil::CopyObjectDefault, _1, _2, _3, _4),
			boost::bind(&JsonUtil::CopyArrayDefault, _1, _2, _3, _4)
			);
	}

	/*
	* 函数名称： JsonStringCopy
	* 使用说明： Json字符串拷贝
	* 输入参数<strSrc>： JSON风格的字符串输入
	* 输入参数<strNew>： JSON风格的字符串输出
	* 输出参数<CopyE>： 元素复制回调函数，函数原型：CopyEle， 回调对象必须是boost::bind()创建
	* 输出参数<CopyO>： 对象复制回调函数，函数原型：CopyObj， 回调对象必须是boost::bind()创建
	* 输出参数<CopyA>： 数组复制回调函数，函数原型：CopyAry， 回调对象必须是boost::bind()创建
	* 返回值: true-成功 false-失败
	*/
	template<typename E, typename O, typename A>
	bool JsonStringCopy(const string& strSrc, string& strNew, const E& CopyE, const O& CopyO, const A& CopyA)
	{
		Json::Value jSrc, jNew;
		Json::Reader jReader;

		if (!jReader.parse(strSrc, jSrc))
			return false;

		if (!JsonObjectCopy(jSrc, jNew, CopyE, CopyO, CopyA))
			return false;

		Json::FastWriter JWriter;
		strNew = JWriter.write(jNew);
		return true;
	}

	template<typename E, typename O>
	bool JsonStringCopy(const string& strSrc, string& strNew, const E& CopyE, const O& CopyO)
	{
		return JsonStringCopy(strSrc, strNew, CopyE, CopyO,
			boost::bind(&JsonUtil::CopyArrayDefault, _1, _2, _3, _4)
			);
	}

	template<typename E>
	bool JsonStringCopy(const string& strSrc, string& strNew, const E& CopyE)
	{
		return JsonStringCopy(strSrc, strNew, CopyE,
			boost::bind(&JsonUtil::CopyObjectDefault, _1, _2, _3, _4),
			boost::bind(&JsonUtil::CopyArrayDefault, _1, _2, _3, _4)
			);
	}
}
