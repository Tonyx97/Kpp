#include <defs.h>

#include <ir/ir.h>

#include "memory.h"
#include "registers.h"
#include "arithmetic.h"

using namespace kpp;

x64::instruction_list x64::generate_memory_op(ir::Instruction* i)
{
	if (auto value_int = rtti::safe_cast<ir::ValueInt>(i))
		return gen_mov(value_int->op1, value_int->op2);
	else if (auto alias = rtti::safe_cast<ir::Alias>(i))
		return gen_mov(alias->op1, alias->op2);
	else if (auto load = rtti::safe_cast<ir::Load>(i))
		return gen_mov(load->op1, load->op2_i->get_value());
	else if (auto store = rtti::safe_cast<ir::Store>(i))
		return gen_mov(store->op1, store->op2_i->get_value());

	return {};
}

x64::instruction_list x64::gen_mov(ir::Value* op1, ir::Value* op2)
{
	const auto& op1_storage = op1->storage,
				op2_storage = op2->storage;

	if (!op1_storage.r)
		return {};

	const auto op2_storage_type = op2->storage_type;

	if (op2_storage_type == ir::STORAGE_INTEGER && op2_storage.integer.u64 == 0)
		return gen_xor(op1_storage.r, op1_storage.r);

	return gen_mov(op1_storage.r, op2);
}

x64::instruction_list x64::gen_mov(reg* r, ir::Value* op, bool inverse)
{
	const auto& op_storage = op->storage;

	const auto op_storage_type = op->storage_type;

	auto ie = new Instruction();

	if (op_storage_type == ir::STORAGE_INTEGER)
	{
		uint8_t opcode = 0xB8 + (r->id % R8);
		
		if (r->id >= R8)
			ie->set_rex(0, 0, 0, 1);

		ie->add_opcode(opcode);
		ie->set_imm(op_storage.integer.u64, 4);
	}
	else if (op_storage_type == ir::STORAGE_REGISTER && r->id != op_storage.r->id)
	{
		ie->add_opcode(0x89);

		if (inverse)
		{
			if (r->id >= R8)
				ie->set_rex(0, 1, 0, 0);

			ie->set_modrm(op_storage.r->id % R8, r->id % R8, 0b11);
		}
		else
		{
			if (op_storage.r->id >= R8)
				ie->set_rex(0, 1, 0, 0);

			ie->set_modrm(r->id % R8, op_storage.r->id % R8, 0b11);
		}
	}

	ie->generate();

	return { ie };
}