#include <defs.h>

#include <ir/ir.h>

#include "built_in.h"
#include "registers.h"
#include "memory.h"

using namespace kpp;

std::unordered_map<std::string, x64::built_in_fn_t> g_built_in_fns =
{
	{ "__rdtsc", x64::gen_rdtsc },
	{ "__int3", x64::gen_int3 },
};

x64::instruction_list x64::generate_built_in_fn(ir::Call* i)
{
	auto prototype = i->prototype;

	if (auto it = g_built_in_fns.find(prototype ? prototype->name : i->name); it != g_built_in_fns.end())
		return it->second(i);

	return {};
}

x64::instruction_list x64::gen_rdtsc(ir::Call* i)
{
	auto ie = new Instruction(i);

	auto value = i->get_value();

	ie->add_opcodes(0x0F, 0x31);
	ie->generate();

	gen_mov(get_reg(RAX), value, true);

	return { ie };
}

x64::instruction_list x64::gen_int3(ir::Call* i)
{
	return { new Instruction(0xCC) };
}

bool x64::is_built_in_fn(const std::string& name)
{
	return g_built_in_fns.contains(name);
}