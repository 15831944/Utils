#pragma once


#ifdef WIN32
#   ifdef UTILS_EXPORTS
#       define UTILT_API __declspec(dllexport)
#   else
#       define UTILT_API __declspec(dllimport)
#   endif //UTILS_EXPORTS
#else
#   define UTILT_API
#endif //WIN32
