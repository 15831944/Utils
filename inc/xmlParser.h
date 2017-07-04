#pragma once
#include <map>
#include <boost/shared_ptr.hpp>
class CXMLNode;

typedef boost::shared_ptr<CXMLNode> CXMLNodeSPtr;
typedef std::map<std::string, std::string> AttrMap;
typedef std::map<std::string, CXMLNode> XmlNodeMap;

const std::string NULL_STR="";

class CXMLNode
{
	public:
		CXMLNode()
		{

		}
		~CXMLNode() throw()
		{

		}

		const CXMLNode & operator [](const std::string &strName)
		{
			XmlNodeMap::iterator iter = m_nodeMap.find(strName);
			if (iter != m_nodeMap.end())
			{
				return iter->second;
			}
			else
			{
				throw;
			}
		}



		int ChildCount()
		{
			return m_nodeMap.size();
		}

	protected:
		AttrMap m_attrMap;
		XmlNodeMap m_nodeMap;
};
