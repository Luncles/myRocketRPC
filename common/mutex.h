#ifndef MYROCKETRPC_COMMON_MUTEX_H
#define MYROCKETRPC_COMMON_MUTEX_H

/*
 * 配置一些锁，保证线程安全
 */

#include <pthread.h>

namespace myRocket {

// RAII，防止退出时忘记解锁    
template <class T>
class ScopeMutex {
public:
    ScopeMutex(T& mutex) : myMutex(mutex) {
        myMutex.lock();
        isMutexLock = true;
    }

    ~ScopeMutex() {
        myMutex.unlock();
        isMutexLock = false;
    }

    void lock() {
        if (!isMutexLock) {
            myMutex.lock();
        }
    }

    void unlock() {
        if (isMutexLock) {
            myMutex.unlock();
        }
    }

private:
    T& myMutex;
    bool isMutexLock {false};
};

/*
 * 互斥锁类：封装了POSIX的互斥锁
 */
class pMutex {
public:
    pMutex() {
        pthread_mutex_init(&myPthreadMutex, nullptr);
    }

    ~pMutex() {
        pthread_mutex_destroy(&myPthreadMutex);
    }

    void lock() {
        pthread_mutex_lock(&myPthreadMutex);
    }

    void unlock() {
        pthread_mutex_unlock(&myPthreadMutex);
    }

    pthread_mutex_t* GetMutex() {
        return &myPthreadMutex;
    }
private:
    pthread_mutex_t myPthreadMutex;
};

}


#endif