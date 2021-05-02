#pragma once

#include "instruction.h"

namespace kpp
{
	namespace x64
	{
		Instruction* gen_label();

		void reset_label_count();
	}
}