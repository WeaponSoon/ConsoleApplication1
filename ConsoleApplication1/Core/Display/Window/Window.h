#pragma once
#include "Core/RHI/RHISurface.h"

class SCWindow : public SCObject
{
private:
	SSPtr<SCRHISurface> surface;


	void internal_key_func(int key, int scancode, int action, int mods);
	void internal_mouse_func(double xposIn, double yposIn);
	void internal_pre_resize_func(int x, int y);
	void internal_resize_func(int x, int y);
	void internal_close_func();

	uint32_t now_x = 0;
	uint32_t now_y = 0;

public:
	SCWindow()
	{
		surface = SCRHISurface::get_surface_creator()();

	}

	virtual void key_func(int key, int scancode, int action, int mods){}
	virtual void mouse_func(double xposIn, double yposIn){}
	virtual void pre_resize_func(int x, int y){}
	virtual void resize_func(int x, int y);
	virtual void close_func(){}

	void init(const std::string& in_title, uint32_t in_width, uint32_t in_height)
	{
		surface->key_callback.bind(SSPtr<SCWindow>(this), &SCWindow::internal_key_func);
		surface->mouse_callback.bind(SSPtr<SCWindow>(this), &SCWindow::internal_mouse_func);
		surface->resize_callback.bind(SSPtr<SCWindow>(this), &SCWindow::internal_resize_func);
		surface->pre_resize_callback.bind(SSPtr<SCWindow>(this), &SCWindow::internal_pre_resize_func);
		surface->close_callback.bind(SSPtr<SCWindow>(this), &SCWindow::internal_close_func);

		now_x = in_width;
		now_y = in_height;
		surface->init(SCRHIInterface::cur_rhi(), in_width, in_width);
		surface->set_title(in_title);
	}

	void resize(uint32_t x, uint32_t y);

	std::tuple<uint32_t, uint32_t> get_size() const { return std::make_tuple(now_x, now_y); }
	std::tuple<uint32_t, uint32_t> get_real_size() const { return surface->get_size(); }

	bool is_valid() const
	{
		return surface->is_valid();
	}

};