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
	virtual SERHIStatus status() const = 0;
	virtual void init() = 0;
	virtual void uninit() = 0;
};