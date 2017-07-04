/***********************************************************************
ģ����	: Utils
�ļ���	: exception.h
����		: DW
��������	: 2017/1/9
��ע		: �쳣��װ
**************************************************************************/
#pragma once

#include "export.h"
#include <exception>
namespace Utils
{
    class CException : public std::exception
    {
    public:
        CException(const char* strException)
            : m_strException(strException)
        {

        }

        virtual ~CException() throw ()
        {

        }

        virtual const char* what()
        {
            return m_strException;
        }

    private:
        const char* m_strException;
    };
}


