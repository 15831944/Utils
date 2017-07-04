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
    //获得当前进程的运行路径
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
        //获取当前目录绝对路径
        if (NULL == getcwd(szCwd, 512))
            return false;

        szCwd[strlen(szCwd)] = '/';
#endif
        return szCwd;
    }

    //内部辅助函数
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
        //如果只有文件名
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

    //传入一个文件路径字符串，将字符串分离为 路径和文件名，以最后一个'\'or'/'为分界点。
    void SplitFilePath(const char* pFullFilePath, char*& pPath, char*& pName)
    {
        static std::string strPath;
        static std::string strName;
        InnerSplitFilePath(pFullFilePath, strPath, strName);
        pPath = (char*)strPath.data();
        pName = (char*)strName.data();
    }

    //内部辅助函数
    int CrossAccess(const char* pFilePath)
    {
#ifdef WIN32
        return _access(pFilePath, 0);
#else
        return access(pFilePath, F_OK);
#endif
    }

    //判断传入的路径是否存在，
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
        if (strPath.size() == 0)return true;//默认当前路径，返回true
        nRet = CrossAccess(strPath.c_str());
        if (0 == nRet)return true;

        return false;
    }
}

