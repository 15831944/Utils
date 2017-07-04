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
	* �������ƣ� CopyAry(�ص�)
	* ʹ��˵���� ���鸴��
	* �������<nIndex>�� Դ������±�
	* �������<jSrc>�� Դ����
	* �������<nNewIndex>�� ����������±�
	* �������<jNew>�� ���������Array
	* ����ֵ: true-�Ѵ��������������Ա������jNew�� false-δ���������Ա������jNew
	*/
	typedef bool(*CopyAry)(size_type nIndex, const Json::Value& jSrc, size_type& nNewIndex, Json::Value& jNew);

	/*
	* �������ƣ� CopyObj(�ص�)
	* ʹ��˵���� ������
	* �������<srcKey>�� Դ�����KEY
	* �������<jSrc>�� Դ����
	* �������<newKey>�� ���������KEY
	* �������<jNew>�� ���������Parent
	* ����ֵ: true-�Ѵ��������������Ա������jNew�� false-δ���������Ա������jNew
	*/
	typedef bool(*CopyObj)(const string& srcKey, const Json::Value& jSrc, string& newKey, Json::Value& jNew);

	/*
	* �������ƣ� CopyEle(�ص�)
	* ʹ��˵���� Ԫ�ظ���
	* �������<srcKey>�� jSrcԴ�����KEY
	* �������<srcValue>�� Դ�����ֵ(����ֵ����ת����string)
	* �������<newKey>�� ���������KEY
	* �������<jNew>�� ���������Parent
	* ����ֵ: true-�Ѵ��������������Ա������jNew�� false-δ���������Ա������jNew
	*/
	typedef bool(*CopyEle)(const string& srcKey, const string& srcValue, string& newKey, Json::Value& jNew);

	//Ĭ�����鸴�ƺ���
	static inline 
	bool CopyArrayDefault(size_type nIndex, const Json::Value& jSrc, size_type& nNewIndex, Json::Value& jNew)
	{
		nNewIndex = nIndex;
		//Ӧ�ñ��⽫jSrc����ֱ�ӿ�����jNew���棬���������ظ�
		jNew[nNewIndex] = Json::Value();
		return true;
	}
	//Ĭ�϶����ƺ���
	static inline 
	bool CopyObjectDefault(const string& srcKey, const Json::Value& jSrc, string& newKey, Json::Value& jNew)
	{
		newKey = srcKey;
		//Ӧ�ñ��⽫jSrc����ֱ�ӿ�����jNew���棬���������ظ�
		jNew[newKey] = Json::Value();
		return true;
	}
	//Ĭ��Ԫ�ظ��ƺ���
	static inline  
	bool CopyElementDefault(const string& srcKey, const string& srcValue, string& newKey, Json::Value& jNew)
	{
		newKey = srcKey;
		jNew[newKey] = srcValue;
		return true;
	}
    //���ں�JsonArrayCopy������ã��߰汾��GCC����ʧ�ܣ�����ǰ������
	template<typename E, typename O, typename A>
	bool JsonObjectCopy(const Json::Value& jSrc, Json::Value& jNew, const E& CopyE, const O& CopyO, const A& CopyA);
	
	template<typename E, typename O, typename A>
	bool JsonArrayCopy(const Json::Value& jArray, Json::Value& jNew, const E& CopyE, const O& CopyO, const A& CopyA)
	{
		//ѭ������JSON����
		for (size_type i = 0; i < jArray.size(); ++i)
		{
			size_type nNewIndex = 0;
			const Json::Value& jValue = jArray[i];
			if (CopyA(i, jValue, nNewIndex, jNew))
			{
				//��ȡ�ص����д����������Ա����
				Json::Value& jNewObject = jNew[nNewIndex];
				//������صĶ���type����null����object��������׳��쳣
				if (Json::nullValue == jNewObject.type() || Json::objectValue == jNewObject.type())
				{
					//����ݹ�
                    JsonObjectCopy<E, O, A>(jValue, jNewObject, CopyE, CopyO, CopyA);
				}
			}
		}

		return true;
	}

	/*
	* �������ƣ� JsonObjectCopy
	* ʹ��˵���� Json���󿽱�
	* �������<jSrc>�� JSON�������
	* �������<jNew>�� JSON�������
	* �������<CopyE>�� Ԫ�ظ��ƻص�����������ԭ�ͣ�CopyEle�� �ص����������boost::bind()����
	* �������<CopyO>�� �����ƻص�����������ԭ�ͣ�CopyObj�� �ص����������boost::bind()����
	* �������<CopyA>�� ���鸴�ƻص�����������ԭ�ͣ�CopyAry�� �ص����������boost::bind()����
	* ����ֵ: true-�ɹ� false-ʧ��
	*/
	template<typename E, typename O, typename A>
	bool JsonObjectCopy(const Json::Value& jSrc, Json::Value& jNew, const E& CopyE, const O& CopyO, const A& CopyA)
	{
		//ѭ������JSON����
		Json::Value::Members jMembers = jSrc.getMemberNames();
		Json::Value::Members::const_iterator iter = jMembers.begin();

		for (; iter != jMembers.end(); iter++)
		{
			const Json::Value& jValue = jSrc[*iter];

			switch (jValue.type())
			{
			case Json::arrayValue:
			{
				//�����������
				string strNewKey;
				//����ص����п����˵�ǰ���������������������飬����������ǰ����������ӽڵ�
				if (CopyO(*iter, jValue, strNewKey, jNew))
				{	
					Json::Value& jNewObject = jNew[strNewKey];
					//������صĶ���type����null����array��������׳��쳣
					if (Json::nullValue == jNewObject.type() || Json::arrayValue == jNewObject.type())
					{
						//�������ݹ�
                        JsonArrayCopy<E, O, A>(jValue, jNewObject, CopyE, CopyO, CopyA);
					}		
				}
				break;
			}
			case Json::objectValue:
			{
				//�����������
				string strNewKey;
				//����ص����п����˵�ǰ��������������ö��󣬷���������ǰ����������ӽڵ�
				if (CopyO(*iter, jValue, strNewKey, jNew))
				{
					Json::Value& jNewObject = jNew[strNewKey];
					//������صĶ���type����null����object��������׳��쳣
					if (Json::nullValue == jNewObject.type() || Json::objectValue == jNewObject.type())
					{
						//����ݹ�
                        JsonObjectCopy<E, O, A>(jValue, jNewObject, CopyE, CopyO, CopyA);
					}
				}
				break;
			}
			//�����л���Ԫ��ת����stringȻ����п���
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
	* �������ƣ� JsonStringCopy
	* ʹ��˵���� Json�ַ�������
	* �������<strSrc>�� JSON�����ַ�������
	* �������<strNew>�� JSON�����ַ������
	* �������<CopyE>�� Ԫ�ظ��ƻص�����������ԭ�ͣ�CopyEle�� �ص����������boost::bind()����
	* �������<CopyO>�� �����ƻص�����������ԭ�ͣ�CopyObj�� �ص����������boost::bind()����
	* �������<CopyA>�� ���鸴�ƻص�����������ԭ�ͣ�CopyAry�� �ص����������boost::bind()����
	* ����ֵ: true-�ɹ� false-ʧ��
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
