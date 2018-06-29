#include <kern/debug.h>
#include <kern/locks.h>
#include <kern/thread.h>
#include <kern/assert.h>
#include <libkern/OSAtomic.h>

#include "PrjFSCommon.h"
#include "Locks.hpp"

static lck_grp_t* s_lockGroup = nullptr;

kern_return_t Locks_Init()
{
    if (nullptr != s_lockGroup)
    {
        return KERN_FAILURE;
    }
    
    s_lockGroup = lck_grp_alloc_init(PrjFSKextBundleId, LCK_GRP_ATTR_NULL);
    if (nullptr == s_lockGroup)
    {
        return KERN_FAILURE;
    }
    
    return KERN_SUCCESS;
}

kern_return_t Locks_Cleanup()
{
    if (nullptr != s_lockGroup)
    {
        lck_grp_free(s_lockGroup);
        s_lockGroup = nullptr;
        
        return KERN_SUCCESS;
    }
    
    return KERN_FAILURE;
}

// Mutex implementation functions

Mutex Mutex_Alloc()
{
    return (Mutex){ lck_mtx_alloc_init(s_lockGroup, LCK_ATTR_NULL) };
}

void Mutex_FreeMemory(Mutex* mutex)
{
    lck_mtx_free(mutex->p, s_lockGroup);
    mutex->p = nullptr;
}

bool Mutex_IsValid(Mutex mutex)
{
    return mutex.p != nullptr;
}

void Mutex_Acquire(Mutex mutex)
{
    lck_mtx_lock(mutex.p);
}

void Mutex_Release(Mutex mutex)
{
    lck_mtx_unlock(mutex.p);
}

// RWLock implementation functions

RWLock RWLock_Alloc()
{
    return (RWLock){ lck_rw_alloc_init(s_lockGroup, LCK_ATTR_NULL) };
}

bool RWLock_IsValid(RWLock rwLock)
{
    return rwLock.p != nullptr;
}

void RWLock_FreeMemory(RWLock* rwLock)
{
    lck_rw_free(rwLock->p, s_lockGroup);
    rwLock->p = nullptr;
}

void RWLock_AcquireShared(RWLock& rwLock)
{
    lck_rw_lock_shared(rwLock.p);

#if PRJFS_LOCK_CORRECTNESS_CHECKS
    assert(rwLock.exclOwner == nullptr);
    OSAddAtomic(1, &rwLock.sharedOwnersCount);
    uint32_t lowBits = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(current_thread()));
    OSBitXorAtomic(lowBits, &rwLock.sharedOwnersXor);
#endif
}

void RWLock_ReleaseShared(RWLock& rwLock)
{
#if PRJFS_LOCK_CORRECTNESS_CHECKS
    uint32_t lowBits = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(current_thread()));
    OSBitXorAtomic(lowBits, &rwLock.sharedOwnersXor);
    OSAddAtomic(-1, &rwLock.sharedOwnersCount);
    assert(rwLock.exclOwner == nullptr);
#endif

    lck_rw_unlock_shared(rwLock.p);
}

void RWLock_AcquireExclusive(RWLock& rwLock)
{
    lck_rw_lock_exclusive(rwLock.p);

#if PRJFS_LOCK_CORRECTNESS_CHECKS
    assert(rwLock.sharedOwnersCount == 0);
    assert(rwLock.sharedOwnersXor == 0);
    rwLock.exclOwner = current_thread();
#endif
}

void RWLock_ReleaseExclusive(RWLock& rwLock)
{
#if PRJFS_LOCK_CORRECTNESS_CHECKS
    assert(rwLock.sharedOwnersCount == 0);
    assert(rwLock.sharedOwnersXor == 0);
    assert(rwLock.exclOwner == current_thread());
    rwLock.exclOwner = nullptr;
#endif

    lck_rw_unlock_exclusive(rwLock.p);
}

bool RWLock_AcquireSharedToExclusive(RWLock& rwLock)
{
#if PRJFS_LOCK_CORRECTNESS_CHECKS
    uint32_t lowBits = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(current_thread()));
    OSBitXorAtomic(lowBits, &rwLock.sharedOwnersXor);
    OSAddAtomic(-1, &rwLock.sharedOwnersCount);
    assert(rwLock.exclOwner == nullptr);
#endif

    bool success = lck_rw_lock_shared_to_exclusive(rwLock.p);
    
#if PRJFS_LOCK_CORRECTNESS_CHECKS
    if (success)
    {
        assert(rwLock.sharedOwnersCount == 0);
        assert(rwLock.sharedOwnersXor == 0);
        rwLock.exclOwner = current_thread();
    }
#endif
    
    return success;
}


