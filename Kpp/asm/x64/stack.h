#pragma once

#include "instruction.h"

namespace kpp
{
	namespace x64
	{
		instruction_list gen_push(reg* r);
		instruction_list gen_pop(reg* r);
	}
}