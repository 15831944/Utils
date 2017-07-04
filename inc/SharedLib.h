/***********************************************************************
模块名	: Utils
文件名	: SharedLib.h
作者		: ZQJ
创建日期	: 2016/12/28
备注		: 实现动态加载动态库接口的类实现；实现跨平台处理。
              NOTE：目前仅实现对C接口的处理。
**************************************************************************/
#pragma once

#include "export.h"
#include <string>
using std::string;

namespace Utils
{
    class UTILT_API CSharedLib
    {
        typedef void* pfnHandle;

    public:
        CSharedLib();
        explicit CSharedLib(const char *libname);
        explicit CSharedLib(const string& libname);
        ~CSharedLib();

    public:
        bool Load(const char *libname);
        bool Load(const string& libname);
        void Unload();
        bool IsLoaded() const;

        bool HasSymbol(const char *symbolname) const;
        bool HasSymbol(const string& symbolname) const;
        pfnHandle GetSymbol(const char *symbolname) const;
        pfnHandle GetSymbol(const string& symbolname) const;

    private:
        pfnHandle m_hInst;
    };

    //example
    //void TestSharedLib()
    //{
    //    //example 1
    //    typedef int(*AddFunc)(int, int);
    //    CSharedLib slib;
    //    if (slib.Load("test.so"))//or "test.dll"
    //    {
    //        
    //        AddFunc pfnAdd = (AddFunc)slib.GetSymbol("testAdd");
    //        int nSum = pfnAdd(1, 2);
    //    }
    //    slib.Unload();//可以不不调用，在析构时会自动调用

    //    //example 2
    //    CSharedLib slib2("test.dll");//or "test.so"
    //    if (slib2.IsLoaded())
    //    {
    //        AddFunc pfnAdd = (AddFunc)slib2.GetSymbol("testAdd");
    //        int nSum = pfnAdd(1, 2);
    //    }
    //    slib2.Unload();//可以不不调用，在析构时会自动调用
    //}
}
