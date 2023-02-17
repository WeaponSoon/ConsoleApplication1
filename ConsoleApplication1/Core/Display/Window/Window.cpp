#include "Window.h"

#include <iostream>

void SCWindow::internal_key_func(int key, int scancode, int action, int mods)
{
	key_func(key, scancode, action, mods);
}

void SCWindow::internal_mouse_func(double xposIn, double yposIn)
{
	mouse_func(xposIn, yposIn);
}

void SCWindow::internal_pre_resize_func(int x, int y)
{
	pre_resize_func(x, y);
}

void SCWindow::internal_resize_func(int x, int y)
{
	now_x = x;
	now_y = y;
	resize_func(x, y);
}

void SCWindow::internal_close_func()
{
	close_func();
}

void SCWindow::resize_func(int x, int y)
{
	std::cout << x << "  " << y << std::endl;
}

void SCWindow::resize(uint32_t x, uint32_t y)
{
	now_x = x;
	now_y = y;
	surface->resize(x, y);
}
