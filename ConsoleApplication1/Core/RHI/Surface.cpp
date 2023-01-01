#include "Core/RHI/Surface.h"

SCDelegate<SSPtr<SCSurface>>& SCSurface::get_surface_creator()
{
	static SCDelegate<SSPtr<SCSurface>> inner;
	return inner;
}
