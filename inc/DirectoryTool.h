/*=========================================================================
描述  ：关于文件路径的工具函数
备注  ：
日期  ：2016.9 xly
=========================================================================*/
#pragma once
#include "export.h"
namespace Utils
{
    /*
    因为链接的MT库，不能在接口中使用string，重要的事情说11遍：
    所有的接口不是线程安全的！
    所有的接口不是线程安全的！
    所有的接口不是线程安全的！
    避免多线程同时调用同一个函数。
    */
    enum PathMode
    {
        E_EXIST_PATH = 0,   //只要路径存在，即使文件不存在也会返回true
        E_EXIST_FILE ,      //必须指定的路径全部存在才会返回true
    };
    //获得当前进程的运行路径(结尾包含路径分隔符)
    UTILT_API const char* GetCurrExeDir();
    //传入一个文件路径字符串，将字符串分离为 路径和文件名，以最后一个'\'or'/'为分界点。
    UTILT_API void SplitFilePath(const char* pFullFilePath, char*& pPath, char*& pName);
    //判断传入的路径是否存在，
    UTILT_API bool PathExist(const char* pFullFilePath, PathMode eMode = E_EXIST_PATH);
}

