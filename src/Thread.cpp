#ifdef WIN32
#   include <Windows.h>
#else
#   include <pthread.h>
#endif

#include "Thread.h"

namespace ThreadUtils
{
#ifdef WIN32
    pthread_id_t GetThisThreadId()
    {
        return GetCurrentThreadId();
    }
#else
    pthread_id_t GetThisThreadId()
    {
        return pthread_self();
    }
#endif //WIN32
}

