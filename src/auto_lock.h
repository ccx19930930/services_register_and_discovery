#ifndef _AUTO_LOCK_H_
#define _AUTO_LOCK_H_

#include <pthread.h>


class CAutoMutexLock
{
public:
    CAutoMutexLock(pthread_mutex_t& mutex)
        : m_mutex(mutex)
    {
        pthread_mutex_lock(&m_mutex);
    }

    ~CAutoMutexLock()
    {
        pthread_mutex_unlock(&m_mutex);
    }

private:
    pthread_mutex_t& m_mutex;
};

#endif
