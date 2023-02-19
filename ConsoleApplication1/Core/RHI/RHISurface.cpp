#include "Core/RHI/RHISurface.h"

SCDelegate<SSPtr<SCRHISurface>>& SCRHISurface::get_surface_creator()
{
	static SCDelegate<SSPtr<SCRHISurface>> inner;
	return inner;
}
