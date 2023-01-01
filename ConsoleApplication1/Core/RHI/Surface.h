#pragma once
#include "Core/CoreObject/SCObject.h"
#include "Core/RHI/RHIInterface.h"
#include "Core/CoreObject/Delegate.h"

class SCSurface : public SCObject
{
public:
	static SCDelegate<SSPtr<SCSurface>>& get_surface_creator();

	SCDelegate<void, int /*key*/, int /*scancode*/, int /*action*/, int /*mods*/> key_callback;
	SCDelegate<void, double /*xposIn*/, double /*yposIn*/> mouse_callback;
	SCDelegate<void, int, int> pre_resize_callback;
	SCDelegate<void, int, int> resize_callback;


	virtual void init(SSPtr<SCRHIInterface> rhi, uint32_t width, uint32_t height) = 0;
	virtual void uninit() = 0;

	virtual bool is_valid() = 0;

	virtual void resize(uint32_t width, uint32_t height) = 0;
	virtual void set_title(const std::string& in_title) = 0;

	virtual std::tuple<uint32_t, uint32_t> get_size() const = 0;
	virtual std::string get_title() const = 0;
};

