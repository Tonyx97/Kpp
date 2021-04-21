#include <defs.h>

#include "x64.h"
#include "registers.h"

using namespace kpp;

void x64::init_arquitecture()
{
	init_registers();
}

void x64::destroy_arquitecture()
{
	destroy_registers();
}