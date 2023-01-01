#include "Core/RHI/RHIInterface.h"

SSPtr<SCRHIInterface>& SCRHIInterface::cur_rhi()
{
	static SSPtr<SCRHIInterface> inner;
	return inner;
}
