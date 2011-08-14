#include "DXUT.h"
#include "fp_thread.h"
#include "CoreDetection/CpuTopology.h"

DWORD WINAPI fp_Thread_ThreadFunc(LPVOID Data) {
    fp_Thread_Data* threadData = (fp_Thread_Data*) Data;
    while(true) {
        // Signal that thread is ready for working
        SetEvent(threadData->m_ReadyForNextJobEvent);
        // Wait till there's a job to be done
        WaitForSingleObject(threadData->m_JobAvailableEvent, INFINITE);
        // Do the job
        threadData->m_JobFunction(threadData->m_JobData);
        // Signal that the job is done
        SetEvent(threadData->m_JobFinishedEvent);
    }
    return 0;
}

fp_WorkerThread::fp_WorkerThread() {
    // Using auto-reset-events
    // All events starts non-signaled
    m_ThreadData.m_ReadyForNextJobEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_ThreadData.m_JobAvailableEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_ThreadData.m_JobFinishedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    m_SysThread = CreateThread(NULL, 0, fp_Thread_ThreadFunc, &m_ThreadData, NULL, 0);
    ResumeThread(m_SysThread);
}

fp_WorkerThread::~fp_WorkerThread() {
    TerminateThread(m_SysThread,0);
    CloseHandle(m_ThreadData.m_ReadyForNextJobEvent);
    CloseHandle(m_ThreadData.m_JobAvailableEvent);
    CloseHandle(m_ThreadData.m_JobFinishedEvent);
}

void fp_WorkerThread::DoJob(void (JobFunction)(void*), void* JobData) {
    // Wait until worker thread is available
    WaitForSingleObject(m_ThreadData.m_ReadyForNextJobEvent, INFINITE);

    // Assign job
    m_ThreadData.m_JobData = JobData;
    m_ThreadData.m_JobFunction = JobFunction;

    // Signal the worker thread that there's a job to be done
    SetEvent(m_ThreadData.m_JobAvailableEvent);
}

void fp_WorkerThread::WaitTillJobFinished() {
    WaitForSingleObject(m_ThreadData.m_JobFinishedEvent, INFINITE);
}

fp_WorkerThreadManager::fp_WorkerThreadManager(int NumWorkerThreads) {
    int numThreads;
    if(NumWorkerThreads >= 1)
        numThreads = NumWorkerThreads;
    else {
        CpuTopology cpuTopo;
        DWORD numCpuCores = cpuTopo.NumberOfSystemCores();
        numThreads = numCpuCores;
    }
    m_WorkerThreads = new fp_WorkerThread[numThreads];
    m_NumWorkerThreads = numThreads;
    m_JobFinishedEvents = new HANDLE[numThreads];
    for(int iWorker=0; iWorker < numThreads; iWorker++) {
        m_JobFinishedEvents[iWorker]
                = m_WorkerThreads[iWorker].m_ThreadData.m_JobFinishedEvent;
    }
}

fp_WorkerThreadManager::~fp_WorkerThreadManager() {
    delete[] m_WorkerThreads;
    delete[] m_JobFinishedEvents;
}

void fp_WorkerThreadManager::DoJobOnAllThreads(
        void (*JobFunction)(void*), 
        void* JobDataArray,
        SIZE_T JobDataSize) {
    for(int iWorker=0; iWorker < m_NumWorkerThreads; iWorker++) {
        m_WorkerThreads[iWorker].DoJob(JobFunction,
                (void*)((int)JobDataArray + iWorker * JobDataSize));
    }
}

void fp_WorkerThreadManager::WaitTillJobFinishedOnAllThreads() {
    WaitForMultipleObjects(m_NumWorkerThreads, m_JobFinishedEvents, TRUE, INFINITE);
}
