//
// Created by chenbaiyi on 18-6-25.
//

#ifndef GCNETCORE_GCCONDBINDMUTEX_H
#define GCNETCORE_GCCONDBINDMUTEX_H

#include <pthread.h>

class GCCondBindMutex
{
public:
    GCCondBindMutex();
    ~GCCondBindMutex();

    bool Wait(int n_timeout = 0);
    void WakeAll();
    void WakeOne();
    void Lock();
    bool TryLock();
    void Unlock();

private:
    pthread_cond_t m_cond;
    pthread_mutex_t m_mutex;
};


#endif //GCNETCORE_GCCONDBINDMUTEX_H
