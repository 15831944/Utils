/***********************************************************************
模块名	: Utils
文件名	: exception.h
作者		: DW
创建日期	: 2017/1/9
备注		: 异常封装(在WINDOWS平台使用此类需要)
**************************************************************************/

#pragma once
#define USING_CRT_SECURE_NO_WARNINGS_WINDOWS 
#ifdef USING_CRT_SECURE_NO_WARNINGS_WINDOWS

#ifndef _CRT_SECURE_NO_WARNINGS
#   define _CRT_SECURE_NO_WARNINGS 
#endif

#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <exception>

#ifdef WIN32
#   pragma warning(disable:4996)
#   define vsnprintf _vsnprintf
#endif //WIN32

namespace Utils
{
    class CWorkException : public std::exception
    {
    public:
        CWorkException(const char* format, ...)
            : errCode(-1), errString(NULL)
        {
            va_list args;
            va_start(args, format);
            Vsprintf(format, args);
            va_end(args);
        }

        CWorkException(int code, const char* format, ...)
            : errCode(code), errString(NULL)
        {
            va_list args;
            va_start(args, format);
            Vsprintf(format, args);
            va_end(args);
        }

        CWorkException(const CWorkException& rhs)
        {
            Copy(rhs.errCode, rhs.errString);
        }

        CWorkException& operator =(const CWorkException& rhs)
        {
            if (&rhs != this)
            {
                Copy(rhs.errCode, rhs.errString);
            }
            return *this;
        }

        virtual ~CWorkException() throw()
        {
            FreeErrString();
        }

        int getCode() const
        {
            return errCode;
        }

        const char* getString() const
        {
            return errString ? errString : "";
        }

        virtual const char* what() const throw()
        {
            return errString ? errString : "";
        }

    private:
        void Vsprintf(const char* format, va_list args)
        {
            if (NULL == format)
            {
                return;
            }

            va_list args_copy;
#if defined(_MSC_VER) || defined(__BORLANDC__)
            args_copy = args;
#else
            va_copy(args_copy, args);
#endif

            int bufSize(1024);
            int retSize(-1);

            while (retSize < bufSize)
            {
                if (errString)
                {
                    free(errString);
                }
                errString = static_cast<char *>(malloc(bufSize + 1));

                if (!errString)
                {
                    break;
                }

                if ((retSize = vsnprintf(errString, bufSize, format, args_copy)) < 0)
                {
                    //长度不够，继续申请长度
                    bufSize *= 2;
                    continue;
                }
                break;
            }

            if (retSize > 0)
            {
                errString[retSize] = '\0';
            }
            va_end(args_copy);
            return;
        }

        void Copy(int code, const char* str)
        {
            errCode = code;
            FreeErrString();

            if (str == NULL)
            {
                return;
            }
            int bufSize = static_cast<int>(strlen(str));
            errString = static_cast<char *>(malloc(bufSize + 1));

            if (errString == NULL)
            {
                return;
            }
            memcpy(errString, str, bufSize);
            errString[bufSize] = '\0';
            return;
        }

    private:
        int errCode;
        char* errString;
        void FreeErrString()
        {
            if (errString)
            {
                free(errString);
            }
            errString = NULL;
        }
    };
}

#ifdef USING_CRT_SECURE_NO_WARNINGS_WINDOWS
#   undef _CRT_SECURE_NO_WARNINGS 
#endif  //USING_CRT_SECURE_NO_WARNINGS_WINDOWS


