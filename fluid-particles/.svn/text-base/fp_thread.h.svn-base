#pragma once
#ifndef FP_THREAD_H
#define FP_THREAD_H

#include "DXUT.h"

#include "fp_global.h"

// Internal structure do not use elsewhere
typedef struct {
    void (*m_JobFunction)(void*);
    void* m_JobData;
    HANDLE m_ReadyForNextJobEvent;
    HANDLE m_JobAvailableEvent;
    HANDLE m_JobFinishedEvent;
} fp_Thread_Data;

class fp_WorkerThread {
    friend class fp_WorkerThreadManager;
public:
    fp_WorkerThread();
    ~fp_WorkerThread();

    // Causes the worker to start working on job and immediately returns. Blocks if
    // the thread is already in use.
    void DoJob(void (*JobFunction)(void*), void* JobData);

    void WaitTillJobFinished();
private:
    fp_Thread_Data m_ThreadData;
    HANDLE m_SysThread;    
};

class fp_WorkerThreadManager {
public:
    fp_WorkerThread* m_WorkerThreads;
    int m_NumWorkerThreads;
    HANDLE* m_JobFinishedEvents;

    // If 'NumWorkerThreads == 0' number of physical processor in system is used
    fp_WorkerThreadManager(int NumWorkerThreads=0);
    ~fp_WorkerThreadManager();

    void DoJobOnAllThreads(
            void (*JobFunction)(void*), 
            void* JobDataArray, 
            SIZE_T JobDataSize);
    void WaitTillJobFinishedOnAllThreads();
};

#endif

