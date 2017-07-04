#include "export.h"

namespace ThreadUtils
{
    typedef unsigned long pthread_id_t;
    UTILT_API pthread_id_t GetThisThreadId();
}

