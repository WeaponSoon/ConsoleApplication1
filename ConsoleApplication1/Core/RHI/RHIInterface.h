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
};