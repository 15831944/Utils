/*=========================================================================
����  �������ļ�·���Ĺ��ߺ���
��ע  ��
����  ��2016.9 xly
=========================================================================*/
#pragma once
#include "export.h"
namespace Utils
{
    /*
    ��Ϊ���ӵ�MT�⣬�����ڽӿ���ʹ��string����Ҫ������˵11�飺
    ���еĽӿڲ����̰߳�ȫ�ģ�
    ���еĽӿڲ����̰߳�ȫ�ģ�
    ���еĽӿڲ����̰߳�ȫ�ģ�
    ������߳�ͬʱ����ͬһ��������
    */
    enum PathMode
    {
        E_EXIST_PATH = 0,   //ֻҪ·�����ڣ���ʹ�ļ�������Ҳ�᷵��true
        E_EXIST_FILE ,      //����ָ����·��ȫ�����ڲŻ᷵��true
    };
    //��õ�ǰ���̵�����·��(��β����·���ָ���)
    UTILT_API const char* GetCurrExeDir();
    //����һ���ļ�·���ַ��������ַ�������Ϊ ·�����ļ����������һ��'\'or'/'Ϊ�ֽ�㡣
    UTILT_API void SplitFilePath(const char* pFullFilePath, char*& pPath, char*& pName);
    //�жϴ����·���Ƿ���ڣ�
    UTILT_API bool PathExist(const char* pFullFilePath, PathMode eMode = E_EXIST_PATH);
}

