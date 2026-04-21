#ifndef __SHARED_MUTEX_COMPAT_H__
#define __SHARED_MUTEX_COMPAT_H__

#include <shared_mutex>

class fallback_shared_mutex {
public:
    void lock() {}
    void unlock() {}
    void lock_shared() {}
    void unlock_shared() {}
};

#if defined(__cpp_lib_shared_mutex) && (__cpp_lib_shared_mutex >= 201505L)
using compat_shared_mutex = std::shared_mutex;
#elif defined(__cpp_lib_shared_timed_mutex) && (__cpp_lib_shared_timed_mutex >= 201402L)
using compat_shared_mutex = std::shared_timed_mutex;
#else
using compat_shared_mutex = fallback_shared_mutex;
#endif

#endif // __SHARED_MUTEX_COMPAT_H__
