/***********************************************************************
ģ����	: Utils
�ļ���	: SharedLib.h
����		: ZQJ
��������	: 2016/12/28
��ע		: ʵ�ֶ�̬���ض�̬��ӿڵ���ʵ�֣�ʵ�ֿ�ƽ̨����
              NOTE��Ŀǰ��ʵ�ֶ�C�ӿڵĴ���
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
    //    slib.Unload();//���Բ������ã�������ʱ���Զ�����

    //    //example 2
    //    CSharedLib slib2("test.dll");//or "test.so"
    //    if (slib2.IsLoaded())
    //    {
    //        AddFunc pfnAdd = (AddFunc)slib2.GetSymbol("testAdd");
    //        int nSum = pfnAdd(1, 2);
    //    }
    //    slib2.Unload();//���Բ������ã�������ʱ���Զ�����
    //}
}
