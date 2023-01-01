#pragma once
#include "Core/RHI/Surface.h"

class SCWindow : public SCObject
{
private:
	SSPtr<SCSurface> surface;


public:
	SCWindow()
	{
		surface = SCSurface::get_surface_creator()();
	}


	void init(const std::string& in_title, uint32_t in_width, uint32_t in_height)
	{
		surface->init(SCRHIInterface::cur_rhi(), in_width, in_width);
		surface->set_title(in_title);
	}

	bool is_valid() const
	{
		return surface->is_valid();
	}

};