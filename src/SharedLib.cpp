
#include "SharedLib.h"

#include <stdio.h>
#ifdef WIN32
#include <Windows.h>
#else
#include <dlfcn.h>
#endif

namespace Utils
{
    CSharedLib::CSharedLib() : m_hInst(NULL)
    {
    }

    CSharedLib::CSharedLib(const char *libname) : m_hInst(NULL)
    {
        Load(libname);
    }

    CSharedLib::CSharedLib(const string& libname) : m_hInst(NULL)
    {
        Load(libname);
    }

    CSharedLib::~CSharedLib()
    {
        Unload();
    }

    bool CSharedLib::Load(const char *libname)
    {
        if (NULL == libname)
        {
            return false;
        }
        //若原先加载过Lib，则卸载原先加载的LIB
        Unload();

#ifdef WIN32
        m_hInst = (pfnHandle)LoadLibraryA(libname);
#else
        //RTLD_LAZY/RTLD_NOW/RTLD_BINDING_MASK/RTLD_NOLOAD/RTLD_DEEPBIND
        m_hInst = dlopen(libname, RTLD_NOW);
#endif

        return (NULL != m_hInst);
    }

    bool CSharedLib::Load(const string& libname)
    {
        return Load(libname.c_str());
    }

    void CSharedLib::Unload()
    {
        if (NULL != m_hInst)
        {
#ifdef WIN32
            FreeLibrary((HMODULE)m_hInst);
#else
            dlclose(m_hInst);
#endif

            m_hInst = NULL;
        }
    }

    bool CSharedLib::IsLoaded() const
    {
        return (NULL != m_hInst);
    }

    bool CSharedLib::HasSymbol(const char *symbolname) const
    {
        return (NULL != GetSymbol(symbolname));
    }
    bool CSharedLib::HasSymbol(const string& symbolname) const
    {
        return (NULL != GetSymbol(symbolname));
    }

    CSharedLib::pfnHandle CSharedLib::GetSymbol(const char *symbolname) const
    {
        if (NULL == symbolname || NULL == m_hInst)
        {
            return NULL;
        }

#ifdef WIN32
        return (pfnHandle)GetProcAddress((HMODULE)m_hInst, symbolname);
#else
        pfnHandle pfn = dlsym(m_hInst, symbolname);
        if (NULL != dlerror())
        {
            return NULL;
        }
        return pfn;
#endif
    }

    CSharedLib::pfnHandle CSharedLib::GetSymbol(const string& symbolname) const
    {
        return GetSymbol(symbolname.c_str());
    }

}
