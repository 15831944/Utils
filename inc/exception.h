/***********************************************************************
模块名	: Utils
文件名	: exception.h
作者		: DW
创建日期	: 2017/1/9
备注		: 异常封装
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


