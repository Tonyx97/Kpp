#include <defs.h>

#include "ir.h"
#include "ssa.h"

#include "reg_alloc.h"

using namespace kpp;

bool reg_alloc::allocate_all()
{
	for (const auto& p : ssa.get_prototypes())
	{
	}

	return true;
}