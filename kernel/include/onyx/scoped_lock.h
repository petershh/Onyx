/*
* Copyright (c) 2019 Pedro Falcato
* This file is part of Carbon, and is released under the terms of the MIT License
* check LICENSE at the root directory for more information
*/

#ifndef _ONYX_SCOPED_LOCK_H
#define _ONYX_SCOPED_LOCK_H

template <typename LockType, bool irq_save = false>
class scoped_lock
{
private:
	bool IsLocked;
	LockType *internal_lock;
public:
	void lock()
	{
		if(irq_save)
			internal_lock->LockIrqsave();
		else
			internal_lock->Lock();
		IsLocked = true;
	}

	void unlock()
	{
		if(irq_save)
			internal_lock->UnlockIrqrestore();
		else
			internal_lock->Unlock();
		IsLocked = false;
	}

	scoped_lock(LockType *lock) : internal_lock(lock)
	{
		this->lock();
	}

	~scoped_lock()
	{
		if(IsLocked)
			unlock();
	}
};

class Spinlock;

using scoped_spinlock = scoped_lock<Spinlock>;
using scoped_spinlock_irqsave = scoped_lock<Spinlock, true>;

#endif