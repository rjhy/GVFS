#ifndef Locks_h
#define Locks_h

#if DEBUG
// Enables validation of shared/exclusive lock invariants and owner checking
#define PRJFS_LOCK_CORRECTNESS_CHECKS 1
#endif

#include <mach/kern_return.h>
#include <stdint.h>

typedef struct __lck_mtx_t__ lck_mtx_t;
typedef struct { lck_mtx_t* p; } Mutex;

kern_return_t Locks_Init();
kern_return_t Locks_Cleanup();

Mutex Mutex_Alloc();
void Mutex_FreeMemory(Mutex* mutex);
bool Mutex_IsValid(Mutex mutex);

void Mutex_Acquire(Mutex mutex);
void Mutex_Release(Mutex mutex);

typedef struct __lck_rw_t__ lck_rw_t;
struct thread;

struct RWLock
{
    lck_rw_t* p;
#if PRJFS_LOCK_CORRECTNESS_CHECKS
    struct thread* exclOwner;
    uint32_t sharedOwnersXor;
    uint32_t sharedOwnersCount;
#endif
};

RWLock RWLock_Alloc();
bool RWLock_IsValid(RWLock rwLock);

void RWLock_FreeMemory(RWLock* rwLock);

void RWLock_AcquireShared(RWLock& rwLock);
void RWLock_ReleaseShared(RWLock& rwLock);

void RWLock_AcquireExclusive(RWLock& rwLock);
void RWLock_ReleaseExclusive(RWLock& rwLock);
bool RWLock_AcquireSharedToExclusive(RWLock& rwLock);



#endif /* Locks_h */
