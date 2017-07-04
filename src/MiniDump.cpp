#include "MiniDump.h"

#ifdef WIN32
#   include <Windows.h>
#   include <DbgHelp.h>
#   include <stdlib.h>
#   pragma comment(lib, "dbghelp.lib")
#endif

namespace Utils
{
#ifdef WIN32
    inline bool IsDataSectionNeeded(LPCWSTR pModuleName)
    {
        if (pModuleName == 0)
        {
            return false;
        }

        WCHAR szFileName[_MAX_FNAME] = L"\0";
        _wsplitpath_s(pModuleName, NULL, NULL, NULL, NULL, szFileName, _MAX_FNAME, NULL, NULL);

        if (_wcsicmp(szFileName, L"ntdll") == 0)
            return true;

        return false;
    }

    inline bool CALLBACK MiniDumpCallback(PVOID pParam,const PMINIDUMP_CALLBACK_INPUT pInput, PMINIDUMP_CALLBACK_OUTPUT pOutput)
    {
        if (pInput == 0 || pOutput == 0)
            return false;

        switch (pInput->CallbackType)
        {
        case ModuleCallback:
            if (pOutput->ModuleWriteFlags & ModuleWriteDataSeg)
                if (!IsDataSectionNeeded(pInput->Module.FullPath))
                    pOutput->ModuleWriteFlags &= (~ModuleWriteDataSeg);
        case IncludeModuleCallback:
        case IncludeThreadCallback:
        case ThreadCallback:
        case ThreadExCallback:
            return true;
        default:;
        }

        return false;
    }

    void CreateMiniDump(PEXCEPTION_POINTERS pep, LPCWSTR strFileName)
    {
        HANDLE hFile = CreateFile(strFileName, GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

        if ((hFile != NULL) && (hFile != INVALID_HANDLE_VALUE))
        {
            MINIDUMP_EXCEPTION_INFORMATION mdei;
            mdei.ThreadId = GetCurrentThreadId();
            mdei.ExceptionPointers = pep;
            mdei.ClientPointers = NULL;

            MINIDUMP_CALLBACK_INFORMATION mci;
            mci.CallbackRoutine = (MINIDUMP_CALLBACK_ROUTINE)MiniDumpCallback;
            mci.CallbackParam = 0;

            ::MiniDumpWriteDump(::GetCurrentProcess(), ::GetCurrentProcessId(), hFile, MiniDumpNormal, (pep != 0) ? &mdei : 0, NULL, &mci);

            CloseHandle(hFile);
        }
    }

    LONG __stdcall MyUnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
    {
        CreateMiniDump(pExceptionInfo, L"core.dmp");
        return EXCEPTION_EXECUTE_HANDLER;
    }

	typedef NTSTATUS(NTAPI* fnRtlGetVersion)(PRTL_OSVERSIONINFOW lpVersionInformation);
	bool IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
	{
		RTL_OSVERSIONINFOEXW verInfo = { 0 };
		verInfo.dwOSVersionInfoSize = sizeof(verInfo);

		auto RtlGetVersion = (fnRtlGetVersion)GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "RtlGetVersion");

		if (RtlGetVersion != 0 && RtlGetVersion((PRTL_OSVERSIONINFOW)&verInfo) == 0)
		{
			if (verInfo.dwMajorVersion > wMajorVersion)
				return true;
			else if (verInfo.dwMajorVersion < wMajorVersion)
				return false;

			if (verInfo.dwMinorVersion > wMinorVersion)
				return true;
			else if (verInfo.dwMinorVersion < wMinorVersion)
				return false;

			if (verInfo.wServicePackMajor >= wServicePackMajor)
				return true;
		}

		return false;
	}
	
    // 此函数一旦成功调用，之后对 SetUnhandledExceptionFilter 的调用将无效
    bool DisableSetUnhandledExceptionFilter()
    {
		//void* addr = (void*)GetProcAddress(LoadLibraryW(L"kernel32.dll"),
		//	"SetUnhandledExceptionFilter");

		void* addr = NULL;
		if (IsWindowsVersionOrGreater(10, 0, 0)) //Win10需要做特殊处理(Win10版本号：10.0.xxxxx)
		{
			addr = (void*)GetProcAddress(LoadLibraryW(L"KERNELBASE.DLL"), "SetUnhandledExceptionFilter");
		}
		else
		{
			addr = (void*)GetProcAddress(LoadLibraryW(L"KERNEL32.DLL"), "SetUnhandledExceptionFilter");
		}

        if (NULL != addr)
        {
            unsigned char code[16];
            int size = 0;

            code[size++] = 0x33;
            code[size++] = 0xC0;
            code[size++] = 0xC2;
            code[size++] = 0x04;
            code[size++] = 0x00;

            DWORD dwOldFlag, dwTempFlag;
            VirtualProtect(addr, size, PAGE_READWRITE, &dwOldFlag);
            WriteProcessMemory(GetCurrentProcess(), addr, code, size, NULL);
            VirtualProtect(addr, size, dwOldFlag, &dwTempFlag);
            return true;
        }

        return false;
    }

    bool CMiniDump::InitMiniDump()
    {
        //注册异常处理函数
        LPTOP_LEVEL_EXCEPTION_FILTER i = SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);

        //使SetUnhandledExceptionFilter无效
        return DisableSetUnhandledExceptionFilter();
    }

#else   //WIN32
    bool CMiniDump::InitMiniDump()
    {
        return true;
    }
#endif  //WIN32
}
