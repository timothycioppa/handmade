#include "work_queue.hpp"
#include <stdio.h>

work_queue gQueue;

void initialize_queue() 
{
    gQueue.running = true;
    gQueue.EntryMutex = PTHREAD_MUTEX_INITIALIZER;
    gQueue.CheckWorkAvailableMutex = PTHREAD_MUTEX_INITIALIZER;
    gQueue.WorkAvailable = PTHREAD_COND_INITIALIZER;
}

void addWork(void* data, WorkCallback callback) 
{
    bool workAdded = false;
    pthread_mutex_lock(&gQueue.EntryMutex);
    {
        if (gQueue.numJobs < 256) 
        {
            work_entry & entry = gQueue.Entries[gQueue.workHead];
            entry.data = data;
            entry.callback = callback;
            gQueue.workHead = NEXT_INDEX(gQueue.workHead);
            gQueue.numJobs++;
        
            // wake up worker thread if necessary
            pthread_mutex_lock(&gQueue.CheckWorkAvailableMutex);
            pthread_cond_signal( &gQueue.WorkAvailable );
            pthread_mutex_unlock(&gQueue.CheckWorkAvailableMutex);        
        }
    }
    pthread_mutex_unlock(&gQueue.EntryMutex);
}

bool get_work_entry(work_entry & entry) 
{
    bool workAvailable = false;

    pthread_mutex_lock(&gQueue.EntryMutex);
    if (gQueue.numJobs > 0) 
    {
        work_entry & nextWorkEntry = gQueue.Entries[gQueue.workTail];
        entry.data = nextWorkEntry.data;
        entry.callback = nextWorkEntry.callback;
        gQueue.workTail = NEXT_INDEX(gQueue.workTail);
        gQueue.numJobs--;
        workAvailable = true;
    } 
    pthread_mutex_unlock(&gQueue.EntryMutex);
    
    return workAvailable;
}

void* MainWorkProc(void* threadInfo) 
{
    work_entry entry;
    while (gQueue.running)
    {
        // wait for work to be available
        pthread_mutex_lock(&gQueue.CheckWorkAvailableMutex);
        while (gQueue.numJobs == 0) 
        {
            pthread_cond_wait( &gQueue.WorkAvailable, &gQueue.CheckWorkAvailableMutex);
        }
        pthread_mutex_unlock(&gQueue.CheckWorkAvailableMutex);

        if (get_work_entry(entry)) 
        {
            entry.callback(entry.data);
        } 
    }
    printf("Worker thread terminated\n");
    return 0;
}
