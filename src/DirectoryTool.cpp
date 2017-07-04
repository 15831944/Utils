#include "DirectoryTool.h"
#include <stdio.h>
#include <string>
#include <cstring>
#ifdef WIN32
#include <io.h>
#include <windows.h>
#else
#include <unistd.h>
#endif

const int nUtilMaxPath = 513;
namespace Utils
{
    //��õ�ǰ���̵�����·��
    const char* GetCurrExeDir()
    {
        static char szCwd[nUtilMaxPath] ={ 0 };
        if (szCwd[0]!=0)
        {
            return szCwd;
        }
#ifdef WIN32
        GetModuleFileNameA(NULL, szCwd, 512);
        char* pTmp = szCwd + (strlen(szCwd) - 1);

        while ((pTmp != szCwd) && (*pTmp != '\\'))
        {
            *pTmp = '\0';
            pTmp--;
        }
#else
        //��ȡ��ǰĿ¼����·��
        if (NULL == getcwd(szCwd, 512))
            return false;

        szCwd[strlen(szCwd)] = '/';
#endif
        return szCwd;
    }

    //�ڲ���������
    void InnerSplitFilePath(const std::string& strFullPath, std::string& strPath, std::string& strName)
    {
        strPath = "";
        strName = "";
        if (strFullPath.size() == 0)return;

        std::size_t dwFilePos = strFullPath.rfind('\\');
        if (dwFilePos == std::string::npos)
        {
            dwFilePos = strFullPath.rfind('/');
        }
        //���ֻ���ļ���
        if (dwFilePos == std::string::npos)
        {
            strName = strFullPath;
            return;
        }
        if (dwFilePos == strFullPath.size() - 1)
        {
            strPath = strFullPath;
            return;
        }
        strName = std::string(&strFullPath[dwFilePos + 1], strFullPath.size() - dwFilePos);
        strPath = std::string(&strFullPath[0], dwFilePos + 1);
    }

    //����һ���ļ�·���ַ��������ַ�������Ϊ ·�����ļ����������һ��'\'or'/'Ϊ�ֽ�㡣
    void SplitFilePath(const char* pFullFilePath, char*& pPath, char*& pName)
    {
        static std::string strPath;
        static std::string strName;
        InnerSplitFilePath(pFullFilePath, strPath, strName);
        pPath = (char*)strPath.data();
        pName = (char*)strName.data();
    }

    //�ڲ���������
    int CrossAccess(const char* pFilePath)
    {
#ifdef WIN32
        return _access(pFilePath, 0);
#else
        return access(pFilePath, F_OK);
#endif
    }

    //�жϴ����·���Ƿ���ڣ�
    bool PathExist(const char* pFullFilePath, PathMode eMode)
    {
        if (pFullFilePath == NULL || strlen(pFullFilePath) == 0)
        {
            return false;
        }
        int nRet = CrossAccess(pFullFilePath);
        if (0 == nRet)return true;
        if (eMode == E_EXIST_FILE)return false;

        std::string strPath;
        std::string strName;
        InnerSplitFilePath(pFullFilePath, strPath, strName);
        if (strPath.size() == 0)return true;//Ĭ�ϵ�ǰ·��������true
        nRet = CrossAccess(strPath.c_str());
        if (0 == nRet)return true;

        return false;
    }
}

