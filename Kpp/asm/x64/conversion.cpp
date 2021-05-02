#include <defs.h>

#include <ir/ir.h>

#include "conversion.h"
#include "registers.h"

using namespace kpp;

x64::instruction_list x64::gen_cwd_cdq()
{
	return { new Instruction(0x99) };
}