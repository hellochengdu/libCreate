//
// Created by chenbaiyi on 18-6-25.
//

#include "../include/jf_inc/GCCondBindMutex.h"

GCCondBindMutex::GCCondBindMutex()
{
    //init mutex
    pthread_mutex_init(&m_mutex, nullptr);

    //init cond
    pthread_cond_init(&m_cond, nullptr);
}

GCCondBindMutex::~GCCondBindMutex()
{
}

/*
 *  des: 等待条件变量
 *  param:
 *      n_timeout:　等待的超时时间
 *  ret:
 *      true表示等待成功，false表示等待失败
 */
bool GCCondBindMutex::Wait(int n_timeout)
{
    //call lock
    pthread_mutex_lock(&m_mutex);

    int n_ret = -1;
    if (!n_timeout)
    {
        n_ret = pthread_cond_wait(&m_cond, &m_mutex);
    }
    else
    {
        timespec tm;
        tm.tv_sec = n_timeout;
        tm.tv_nsec = 0;
        n_ret = pthread_cond_timedwait(&m_cond, &m_mutex, &tm);
    }

    if (!n_ret)
    {
        pthread_mutex_unlock(&m_mutex);
        return true;
    }
    else
    {
        return false;
    }
}

/*
 *  des: 加锁
 */
void GCCondBindMutex::Lock()
{
    pthread_mutex_lock(&m_mutex);
}

/*
 *  des: 尝试加锁，
 *  ret:
 *      true表示加锁成功，false表示加锁失败
 */
bool GCCondBindMutex::TryLock()
{
    int n_ret = pthread_mutex_trylock(&m_mutex);
    if (!n_ret)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*
 *  des: 解锁
 */
void GCCondBindMutex::Unlock()
{
    pthread_mutex_unlock(&m_mutex);
}

/*
 *  des: 广播，随机唤醒单个睡入互斥量的条件变量
 */
void GCCondBindMutex::WakeOne()
{
    pthread_cond_signal(&m_cond);
}

/*
 *  des: 广播，唤醒所有睡入互斥量的条件变量
 */
void GCCondBindMutex::WakeAll()
{
    pthread_cond_broadcast(&m_cond);
}
