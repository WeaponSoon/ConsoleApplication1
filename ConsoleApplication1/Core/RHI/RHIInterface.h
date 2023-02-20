#pragma once
#include "Core/CoreObject/SCObject.h"

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

class SCRHICommand : public SCObject
{
public:
	virtual void on_command_record(class SCRHICommandBuffer& cmd_buffer) const = 0;
};

class SCRHICommandBuffer : public SCObject
{
private:

	SSSpinLock record_command_lock;
	SSSpinLock status_lock;

	std::vector<SSPtr<SCRHICommand>> all_command;
	SECommandBufferStatus command_buffer_status = SECommandBufferStatus::Invalid;
	SECommandBufferLifeType command_buffer_life = SECommandBufferLifeType::ExecuteMulti;
public:
	SECommandBufferStatus get_status() const
	{
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
		return false;
	}
	virtual void on_end_record() = 0;


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

	//virtual SSPtr<SCRHICommandBuffer> get_command_buffer() = 0;
};