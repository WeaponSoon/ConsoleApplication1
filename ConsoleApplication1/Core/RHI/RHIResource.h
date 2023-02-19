#pragma once
#include <list>

#include "Core/RHI/RHIInterface.h"

class SCRHIResource : public SCObject
{

private:
	SCRHIResource(const SCRHIResource&) = default;
	SCRHIResource(SCRHIResource&&) = default;


	struct SCRHIResourceLockItem
	{
		mutable bool bHasLocked = false;
		mutable std::list<SSPtr<SCRHIResource>>::const_iterator lockPosition;

		SCRHIResourceLockItem() = default;
		SCRHIResourceLockItem(const SCRHIResourceLockItem&) : SCRHIResourceLockItem() {}
		SCRHIResourceLockItem(SCRHIResourceLockItem&&) : SCRHIResourceLockItem() {}
		SCRHIResourceLockItem& operator=(const SCRHIResourceLockItem&) { return *this; }
		bool operator==(const SCRHIResourceLockItem&) const { return true; }
		bool operator!=(const SCRHIResourceLockItem&) const { return false; }
	};

	SCRHIResourceLockItem lockItem;

	struct SCRHIResourceLockHolder
	{
		void lockResource(const SSPtr<SCRHIResource>& inResource)
		{
			while(operate_flag.test_and_set())
			{}
			if(!inResource->lockItem.bHasLocked)
			{
				inResource->lockItem.bHasLocked = true;
				inResource->lockItem.lockPosition = lockedResources.emplace(lockedResources.end(), inResource);
			}
			operate_flag.clear();
		}

		void unlockResource(const SSPtr<SCRHIResource>& inResource)
		{
			while (operate_flag.test_and_set())
			{}
			if(inResource->lockItem.bHasLocked)
			{
				inResource->lockItem.bHasLocked = false;
				lockedResources.erase(inResource->lockItem.lockPosition);
			}
			operate_flag.clear();
		}

	private:
		std::list<SSPtr<SCRHIResource>> lockedResources;
		std::atomic_flag operate_flag = ATOMIC_FLAG_INIT;

	};
	static SCRHIResourceLockHolder holder;
public:
	SCRHIResource() = default;
	void lock()
	{
		holder.lockResource(this);
	}
	void unlock()
	{
		holder.unlockResource(this);
	}

};