#include <defs.h>

#include <ir/ir.h>

#include "memory.h"
#include "registers.h"
#include "arithmetic.h"

using namespace kpp;

std::vector<x64::Instruction*> x64::generate_memory_op(ir::Instruction* i)
{
	if (auto value_int = rtti::safe_cast<ir::ValueInt>(i))
		return gen_mov(value_int->op1, value_int->op2);
	if (auto alias = rtti::safe_cast<ir::Alias>(i))
		return gen_mov(alias->op1, alias->op2);

	return {};
}

std::vector<x64::Instruction*> x64::gen_mov(ir::Value* op1, ir::Value* op2)
{
	auto ie = new x64::Instruction();

	const auto& op1_storage = op1->storage,
				op2_storage = op2->storage;

	const auto op2_storage_type = op2->storage_type;

	if (op2_storage_type == ir::STORAGE_INTEGER && op2_storage.integer.u64 == 0)
		return gen_xor(op1, op1);

	if (op2_storage_type == ir::STORAGE_INTEGER)
	{
		uint8_t opcode = 0xB8;

		opcode += op1_storage.r->id;

		ie->set_imm(op2_storage.integer.u64, 4);
		ie->add_opcode(opcode);
	}
	else if (op2_storage_type == ir::STORAGE_REGISTER && op1_storage.r != op2_storage.r)
	{
		ie->add_opcode(0x89);
		ie->set_modrm(op1_storage.r->id, op2_storage.r->id, 0b11);
	}

	ie->generate();

	return { ie };
}