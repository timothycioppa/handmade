#ifndef _WORK_QUEUE_JC_HPP
#define _WORK_QUEUE_JC_HPP
#include <pthread.h>
typedef void (*WorkCallback) (void* data);

struct work_entry 
{
    void* data;
    WorkCallback callback;
};

struct work_queue 
{
    pthread_mutex_t EntryMutex;
    pthread_mutex_t CheckWorkAvailableMutex;
    pthread_cond_t   WorkAvailable;
    work_entry Entries[256];
    bool running;
    volatile int workHead;
    volatile int workTail;
    volatile int numJobs;
};

#define NEXT_INDEX(index) ((index) + 1) % 256

extern work_queue gQueue;

void initialize_queue();
void addWork(void* data, WorkCallback callback) ;
bool get_work_entry(work_entry & entry) ;
void* MainWorkProc(void* threadInfo) ;

#endif