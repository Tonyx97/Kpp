#include <defs.h>

#include "registers.h"

using namespace kpp;

std::unordered_map<reg_id, reg*> registers;

void x64::init_registers()
{
	registers.insert({ RAX, new reg { .id = RAX, .general = false, .volatil = true } });
	registers.insert({ RCX, new reg { .id = RCX, .general = false, .volatil = true } });
	registers.insert({ RDX, new reg { .id = RDX, .general = false, .volatil = true } });
	registers.insert({ RBX, new reg { .id = RBX, .general = true,  .volatil = false } });
	registers.insert({ RSP, new reg { .id = RSP, .general = false, .volatil = false } });
	registers.insert({ RBP, new reg { .id = RBP, .general = false, .volatil = false } });
	registers.insert({ RSI, new reg { .id = RSI, .general = true,  .volatil = false } });
	registers.insert({ RDI, new reg { .id = RDI, .general = true,  .volatil = false } });
	registers.insert({ R8,  new reg { .id = R8,  .general = true,  .volatil = true } });
	registers.insert({ R9,  new reg { .id = R9,  .general = true,  .volatil = true } });
	registers.insert({ R10, new reg { .id = R10, .general = true,  .volatil = true } });
	registers.insert({ R11, new reg { .id = R11, .general = true,  .volatil = true } });
	registers.insert({ R12, new reg { .id = R12, .general = true,  .volatil = false } });
	registers.insert({ R13, new reg { .id = R13, .general = true,  .volatil = false } });
	registers.insert({ R14, new reg { .id = R14, .general = true,  .volatil = false } });
	registers.insert({ R15, new reg { .id = R15, .general = true,  .volatil = false } });
}

void x64::destroy_registers()
{
	for (auto [id, r] : registers)
		delete r;
}

void x64::for_each_register(const std::function<register_fn>& fn)
{
	for (auto [id, r] : registers)
		fn(r);
}

reg* x64::get_reg(reg_id id)
{
	auto it = registers.find(id);
	return (it != registers.end() ? it->second : nullptr);
}