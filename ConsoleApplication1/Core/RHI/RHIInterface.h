#pragma once
#include "Core/CoreObject/SCObject.h"
#include "Core/RHI/RHIInterfaceDefs.h"

class SCRHIInterface;

enum class SERHIStatus
{
	NotInit,
	Initing,
	Inited,
	InitFaild,
	Uniniting
};


enum class SECommandBufferStatus
{
	Initial,
	Recording,
	Executable,
	Pending,
	Invalid,
};


enum class SECommandBufferLifeType
{
	ExecuteOnce,
	ExecuteMulti,
};

class SCRHIDeviceFence : public SCObject
{
public:
	virtual bool IsReady() const = 0;
	virtual void Reset() = 0;
};

class SCRHIDeviceSemaphore : public  SCObject
{
	
};

struct SSRHICommandBufferWaitInfo
{
	SERHIPipelineStageFlags m_flags;
	SSPtr<SCRHIDeviceSemaphore> m_semaphore;
};

struct SSRHICommandBufferTriggerInfo
{
	SSPtr<SCRHIDeviceSemaphore> m_semaphore;
};

class SCRHICommand : public SCObject
{
public:
	virtual void on_command_record(class SCRHICommandBuffer& cmd_buffer) const = 0;
};

class SCRHICommandBuffer : public SCObject
{
protected:

	SSPtr<SCRHIDeviceFence> Fence;

	SSSpinLock record_command_lock;
	SSSpinLock status_lock;

	std::vector<SSPtr<SCRHICommand>> all_command;
	SECommandBufferStatus command_buffer_status = SECommandBufferStatus::Invalid;
	SECommandBufferLifeType command_buffer_life = SECommandBufferLifeType::ExecuteMulti;

protected:

	void CheckStatus()
	{
		if (command_buffer_status == SECommandBufferStatus::Pending)
		{
			if (Fence->IsReady())
			{
				Fence->Reset();
				command_buffer_status = get_command_buffer_life() == SECommandBufferLifeType::ExecuteMulti ? SECommandBufferStatus::Executable : SECommandBufferStatus::Invalid;
			}
		}
	}

public:


	SECommandBufferStatus get_status() const
	{
		const_cast<SCRHICommandBuffer*>(this)->CheckStatus();
		return command_buffer_status;
	}
	SECommandBufferLifeType get_command_buffer_life() const
	{
		return command_buffer_life;
	}

	bool begin_record()
	{
		SSScopeSpinLock guard(status_lock);

		if(command_buffer_status != SECommandBufferStatus::Initial)
		{
			return false;
		}
		on_begin_record();
		command_buffer_status = SECommandBufferStatus::Recording;
		return true;
	}
	virtual void on_begin_record() = 0;

	bool end_record()
	{
		SSScopeSpinLock guard(status_lock);

		if(command_buffer_status != SECommandBufferStatus::Recording)
		{
			return false;
		}
		on_end_record();
		command_buffer_status = SECommandBufferStatus::Executable;
		return true;
	}
	virtual void on_end_record() = 0;

	bool submit(const std::vector<SSRHICommandBufferWaitInfo>& InWaitInfo, const std::vector<SSRHICommandBufferTriggerInfo>& InTriggerInfo)
	{
		SSScopeSpinLock guard(status_lock);
		if(command_buffer_status != SECommandBufferStatus::Executable)
		{
			return false;
		}
		on_submit(InWaitInfo, InTriggerInfo);
		command_buffer_status = SECommandBufferStatus::Pending;
		return true;
	}
	virtual void on_submit(const std::vector<SSRHICommandBufferWaitInfo>& InWaitInfo, const std::vector<SSRHICommandBufferTriggerInfo>& InTriggerInfo) = 0;


	bool record_command(const SSPtr<SCRHICommand>& in_command)
	{
		if(command_buffer_status != SECommandBufferStatus::Recording)
		{
			return false;
		}
		SSScopeSpinLock guard(record_command_lock);
		in_command->on_command_record(*this);
		all_command.push_back(in_command);
		return true;
	}

	virtual void wait_until_finish(uint64_t InOutTime = 0xffffffffffffffffull) = 0;

};


class SCRHIInterface : public SCObject
{
public:
	static SSPtr<SCRHIInterface>& cur_rhi();

	void make_current() { cur_rhi() = this; }
	void make_no_current()
	{
		if (cur_rhi().get_raw_ptr() == this)
		{
			cur_rhi() = nullptr;
		}
	}

	virtual SERHIStatus status() const = 0;
	virtual void init() = 0;
	virtual void uninit() = 0;

	virtual SSPtr<SCRHICommandBuffer> allocate_command_buffer() = 0;
	virtual void reset_command_buffer(SSPtr<SCRHICommandBuffer>& InBuffer) = 0;
};